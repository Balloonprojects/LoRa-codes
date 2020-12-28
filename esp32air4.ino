// EDUCADRONE 2020
// Programme fonctionnant avec les cartes TTGO Lora32 V2
// utilisation de la bibliothèque SSD1306 (gestion LCD)
// Station d'analyse de l'air


#include <string.h>
#include <SPI.h>
#include <Wire.h>
#include <RtcDS1307.h>
#include "ccs811.h"  // CCS811 library  CO2
#include "SparkFunBME280.h" // BME280 pression / température
#include "DHT.h"  // Capteur DHT11 température/humidité
#include <BH1750.h> // Capteur GY-30 de luminosité
#include <SDS011.h> // Capteur de particules
#include "SSD1306.h" // Ecran cristaux liquides

#define DHTPIN 23     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
#define UVPIN 15    //broche analogique capteur UV

SSD1306 display(0x3c, 21, 22); // pour carte TTGO Lora32 V2
//SSD1306 display(0x3c, 4, 15); // pour carte HELTEC Lora32

RtcDS1307<TwoWire> Rtc(Wire);
CCS811 ccs811;
BME280 mySensorB; //Uses I2C address 0x76 (jumper closed)
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
SDS011 my_sds;

HardwareSerial port(1);
float p10, p25;
int err;

int Typscen = 0;
int Typbat = 0;

uint32_t heartbeatTime = 0;
uint32_t PulsTime = 0;
uint32_t BtuTime = 0;
uint32_t AffTime = 0;
uint32_t BleTime = 0;

uint16_t BTUsyncr = 33;
uint16_t BTUisyncr;

#define countof(a) (sizeof(a) / sizeof(a[0]))
void Pdt (RtcDateTime& dtm);


void Pdt (RtcDateTime& dtm)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dtm.Month(),
             dtm.Day(),
             dtm.Year(),
             dtm.Hour(),
             dtm.Minute(),
             dtm.Second() );
  Serial.print(datestring);
}


//Returns une moyenne de mesures (ici 8)
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;

  for (int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return (runningValue);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup()
{
  uint32_t curTime;

  delay(2000);

  Serial.begin(115200);

  // Initialisation horloge RTC
  Wire.begin(4, 5); // choix des broches SDA et SCL sur ESP32
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid())
  {
    Serial.println("Erreur de communication avec hormoge RTC");
  }
  if (!Rtc.GetIsRunning())
  {
    Serial.println("Démarrage horloge RTC.");
    Rtc.SetIsRunning(true);
  }
  Serial.print ("Date compilation programme : "); Pdt(compiled); Serial.println();
  RtcDateTime now = Rtc.GetDateTime();
  Serial.print ("Date renvoyée par horloge  : "); Pdt(now); Serial.println();

  // Initialisation BME280 (pression/temp)
  mySensorB.setI2CAddress(0x76);
  if (mySensorB.beginI2C() == false) Serial.println("Sensor B connect failed");
  Serial.print(" HumidityB: ");
  Serial.print(mySensorB.readFloatHumidity(), 0);
  Serial.print(" PressureB: ");
  Serial.print(mySensorB.readFloatPressure(), 0);
  Serial.print(" TempB: ");
  Serial.print(mySensorB.readTempC(), 2);
  //Serial.print(mySensorB.readTempF(), 2);
  Serial.println();

  // Initialisation DHT11
  dht.begin();

  // Initialisation capteur GY-30 luminosité
  lightMeter.begin();

  // Initialisation port série pour capteur de particules
  my_sds.begin(&port, 34, 35); // broches 22 et 23

  // Initialisation des ports séries pour les capteurs d'ozone et de formaldhéyde
  //Serial2.begin(9600, SERIAL_8N1, 32, 33);

  // Initialisation pin UV en lecture
  pinMode(UVPIN, INPUT);

}


int cmpta;

void loop()
{
  uint32_t curTime = millis();
  char mess1[50];
  char mess2[50];
  char mess3[50];
  char mess4[50];
  uint16_t nbs;
  uint16_t eco2, etvoc, errstat, raw;
  uint16_t buf[9];


  curTime = millis();


  if (curTime > PulsTime)
  {
    PulsTime = curTime + 3000;

    //Lecture et affichage horloge
    RtcDateTime now = Rtc.GetDateTime();
    Pdt(now); Serial.println();

    //Lecture et affichage capteur BNE280 (pression/temperature)
    Serial.print("Pression : ");
    Serial.print(mySensorB.readFloatPressure(), 0);
    Serial.println(" Pa");
    sprintf(mess1, "Pression : %.0f Pa", mySensorB.readFloatPressure() );
    Serial.print("Température capteur 1 : ");
    Serial.print(mySensorB.readTempC(), 2);
    Serial.println(" °C");
    sprintf(mess2, "Température : %.1f °C", mySensorB.readTempC() );

    //Lecture et affichage DHT11 temperature/humidité
    float h = dht.readHumidity();
    float t = dht.readTemperature();   //en celsius
    Serial.print("Température capteur 2 : ");
    Serial.print(t);
    Serial.println(" °C");
    Serial.print("Humidité: ");
    Serial.print(h);
    Serial.println(" %");
    sprintf(mess3, "Humidité : %.0f %", h );

    // Lecture et affichage luminosité
    float lux = lightMeter.readLightLevel();
    Serial.print("Luminosité: ");
    Serial.print(lux);
    Serial.println(" lx");
    sprintf(mess4, "Luminosité : %.0f lx", lux );

    // Lecture et affichage taux d'UV (à 25°)
    int uvLevel = averageAnalogRead(UVPIN);
    //float outputVoltage = (uvLevel + 238.84)/1297;
    float outputVoltage = (uvLevel * 3.3) / 4095;
    //float uvIntensity = mapfloat(outputVoltage, 0.96, 2.76, 0.0, 15.0); //T = 75°
    float uvIntensity = mapfloat(outputVoltage, 1, 2.84, 0.0, 15.0); //T = 25°
    //float uvIntensity = mapfloat(outputVoltage, 1.02, 2.9, 0.0, 15.0); //T = -5°
    //float uvIntensity = mapfloat(outputVoltage, 1.03, 2.94, 0.0, 15.0); //T = -25°
    //Serial.print("ML8511 : ");
    Serial.print(" uvlevel: ");
    Serial.print(uvLevel);
    Serial.print(" voltage: ");
    Serial.print(outputVoltage);
    Serial.print("Taux d'UV : ");
    Serial.print(uvIntensity);
    Serial.println(" mW/cm^2");


    // Lecture et affichage taux de particules
    err = my_sds.read(&p25, &p10);
    if (!err)
    {
      Serial.print("Taux de particules P2.5: " + String(p25));
      Serial.println(" ug/m3");
      Serial.print("Taux de particules P10:  " + String(p10));
      Serial.println(" ug/m3");
    }

    //Lecture et affichage taux formaldéhyde et d'ozone
    Serial2.begin(9600, SERIAL_8N1, 32, 33);  // formaldhéyde
    while (!Serial2.available());
    delay(50);
    if (Serial2.available())
    {
      for (int i = 0 ; i < 9 ; i++)
      {
        buf[i] = Serial2.read();
        //Serial.println(buf[i],HEX);
      }
      Serial.print("Concentration formaldéhydes : ");
      //Serial.print ("Identificateur : "); Serial.print(buf[1],HEX);
      //Serial.print ("  Val max : "); Serial.print(buf[6]*256 + buf[7]);
      //Serial.print ("  Val cour : "); Serial.print(buf[4]*256 + buf[5]);
      float ppm = (buf[4] * 256 + buf[5]) / 1000.00;
      Serial.print(ppm); Serial.println(" mg/m3");
    }

    Serial2.begin(9600, SERIAL_8N1, 36, 39);  // ozone
    while (!Serial2.available());
    delay(50);
    if (Serial2.available())
    {
      for (int i = 0 ; i < 9 ; i++)
      {
        buf[i] = Serial2.read();
        //Serial.println(buf[i],HEX);
      }
      Serial.print("Concentration ozone : ");
      //Serial.print ("Identificateur : "); Serial.print(buf[1],HEX);
      //Serial.print ("  Val max : "); Serial.print(buf[6]*256 + buf[7]);
      //Serial.print ("  Val cour : "); Serial.print(buf[4]*256 + buf[5]);
      float ppm = (buf[4] * 256 + buf[5]) / 1000.00;
      Serial.print(ppm); Serial.println(" mg/m3");
    }

    Serial.println();

    //initialisation ecran LCD
    pinMode(16, OUTPUT);
    pinMode(2, OUTPUT);
    digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
    delay(50);
    digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
    display.init();
    display.flipScreenVertically();
    //Affichage écran LCD
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, " EDUCADRONE");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 20, mess1);
    display.drawString(0, 31, mess2);
    display.drawString(0, 42, mess3);
    display.drawString(0, 53, mess4);
    display.display();
    delay(50);
    Wire.begin(4, 5); // choix des broches SDA et SCL sur ESP32
    delay(50);
  }

  if (curTime > BleTime)
  {
    BleTime = curTime + 100;


  }
}
