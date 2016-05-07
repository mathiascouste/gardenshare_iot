#! /usr/bin/python

import httplib, urllib, bluetooth, json

moduleserial = ""
temperature = ""
light = ""
humidity = ""
lat= 0.0
lon= 0.0

headers = {"Accept": "text/plain"}
conn = httplib.HTTPConnection("127.0.0.1:1880")
conn.request("GET", "/geoloc")
response = conn.getresponse()
print response.status, response.reason
data = response.read()
print data
conn.close()

decodedData = json.loads(data)

lat = str(decodedData['LocationRS']['location'][0]['latitude'][0])
lon = str(decodedData['LocationRS']['location'][0]['longitude'][0])

def sendToServer(postPath, postdata):
    headers = {"Content-type": "application/json","Accept": "text/plain"}
    conn = httplib.HTTPConnection("gardenhubconnector-2.eu-gb.mybluemix.net")
    conn.request("POST", postPath, postdata, headers)
    print postdata
    response = conn.getresponse()
    print response.status, response.reason
    data = response.read()
    print data
    conn.close()

def logData( key, value ):
    global moduleserial
    global temperature
    global light
    global humidity
    
    if key.startswith('moduleserial'):
        moduleserial = value
    elif key.startswith('temperature'):
        temperature = value
    elif key.startswith('humidity'):
        humidity = value
    elif key.startswith('light'):
        light = value

def sendsend(postPath,datatype):
    data = "{\"type\":\""+datatype+"\",\"serial\":\""+moduleserial+"\",\"temperature\":\""+temperature+"\",\"light\":\""+light+"\", \"humidity\":\""+humidity+"\", \"latitude\":\""+lat+"\", \"longitude\":\""+lon+"\"}"
    sendToServer(postPath,data)

def register( ):
    print 'register'
    sendsend("register","register")

def senddata( ):
    print 'senddata'
    sendsend("senddata","senddata")

target_suffix = "HC"
target_address = None

print ""
print "Scanning around ..."
nearby_devices = bluetooth.discover_devices()

for bdaddr in nearby_devices:
    if bluetooth.lookup_name( bdaddr ).startswith(target_suffix):
        target_address = bdaddr
        break

if target_address is not None:
    print "found target bluetooth device with address ", target_address
    sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )

    print "Trying connection"
    port = 1
    sock.connect((target_address, port))
    print "Trying sending"
    sock.send("bonjour")
    sock.send('\n')
    print "Finished sending"
    
    line = ''
    l = 1
    while l == 1:
        e = sock.recv(1)
        line += e
        if e == '\n':
            line = line[:-1]
            line = line[:-1]
            print line
            if line.startswith('REGISTER'):
                register()
                l = 0
            elif line.startswith('SENDDATA'):
                senddata()
                l = 0
            else:
                tab = line.split('=', 1 )
                logData(tab[0], tab[1])
            line = ''
    sock.close()
else:
    print "could not find target bluetooth device nearby"
