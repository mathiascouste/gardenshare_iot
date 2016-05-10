
#include <SoftwareSerial.h>

const int SIGN = 1;
String msg = "";
#include <avr/sleep.h>
#include <SoftwareSerial.h>

#define HUM_PIN A5
#define LIGHT_PIN A4
#define LED_PIN 13
#define INT_PIN 2
#define MODULESERIAL "AE0X1234"

#include <OneWire.h> // Inclusion de la librairie OneWire
#define DS18B20 0x28     // Adresse 1-Wire du DS18B20
#define BROCHE_ONEWIRE 7 // Broche utilisée pour le bus 1-Wire

SoftwareSerial blue(10,11);

OneWire ds(BROCHE_ONEWIRE); // Création de l'objet OneWire ds
 
// Fonction récupérant la température depuis le DS18B20
// Retourne true si tout va bien, ou false en cas d'erreur
boolean getTemperature(float *temp){
  byte data[9], addr[8];
  // data : Données lues depuis le scratchpad
  // addr : adresse du module 1-Wire détecté
 
  if (!ds.search(addr)) { // Recherche un module 1-Wire
    ds.reset_search();    // Réinitialise la recherche de module
    return false;         // Retourne une erreur
  }
   
  if (OneWire::crc8(addr, 7) != addr[7]) // Vérifie que l'adresse a été correctement reçue
    return false;                        // Si le message est corrompu on retourne une erreur
 
  if (addr[0] != DS18B20) // Vérifie qu'il s'agit bien d'un DS18B20
    return false;         // Si ce n'est pas le cas on retourne une erreur
 
  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sélectionne le DS18B20
   
  ds.write(0x44, 1);      // On lance une prise de mesure de température
  delay(800);             // Et on attend la fin de la mesure
   
  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sélectionne le DS18B20
  ds.write(0xBE);         // On envoie une demande de lecture du scratchpad
 
  for (byte i = 0; i < 9; i++) // On lit le scratchpad
    data[i] = ds.read();       // Et on stock les octets reçus
   
  // Calcul de la température en degré Celsius
  *temp = ((data[1] << 8) | data[0]) * 0.0625; 
   
  // Pas d'erreur
  return true;
}

float convertToHumidity(int h) {
   if(h < 300) {
    return 0;
   } else if(h < 700) {
     return (((double)h) -300.0)/4.0;
   } else {
     return 100;
   }
}

float convertToLight(int l) {
   return ((double)l)*100.0/1024.0;
}

float temp = 0;
float hum = 0;
float light = 0;

int registerFlag = 0;

void interupt_handler() {
  registerFlag = 1;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(HUM_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
  
  pinMode(INT_PIN, INPUT/*_PULLUP*/);
  //attachInterrupt(INT0, interupt_handler, FALLING);
  
#if defined DEBUG
  Serial.begin( 9600 );
#endif
  blue.begin( 9600 );    // 9600 is the default baud rate for the serial Bluetooth module
}

void registerDevice() {
  blue.print("moduleserial=");
  blue.println(MODULESERIAL);
  blue.println("REGISTER");
  
#if defined DEBUG
  Serial.print("moduleserial=");
  Serial.println(MODULESERIAL);
  Serial.println("REGISTER");
#endif
}


void senddataorregister() {
  if(registerFlag) {
    registerDevice();
    registerFlag = 0; 
  } else {
  while(!getTemperature(&temp));
    hum = convertToHumidity(analogRead(HUM_PIN));
    light = convertToLight(analogRead(LIGHT_PIN));
    
    blue.print("moduleserial=");
    blue.println("AE0X1234");
    blue.print("temperature=");
    blue.println(temp);
    blue.print("humidity=");
    blue.println(hum);
    blue.print("light=");
    blue.println(light);
    blue.println("SENDDATA");
    
#if defined DEBUG
    Serial.print("moduleserial=");
    Serial.println(MODULESERIAL);
    Serial.print("temperature=");
    Serial.println(temp);
    Serial.print("humidity=");
    Serial.println(hum);
    Serial.print("light=");
    Serial.println(light);
    Serial.println("SENDDATA");
    Serial.println("");
#endif
  }
}

void loop() {
  //check for inputs to bluetooth adapter
  while (blue.available()) {
   char reading = blue.read();
    if (reading == '\n') {
      
#if defined DEBUG
      Serial.println(msg);
#endif
      senddataorregister();
      msg = "";
    } else {
      msg += reading;
    }
  }
}
