/***************************************************
  This is an example for the HTU21D-F Humidity & Temp Sensor

  Designed specifically to work with the HTU21D-F sensor from Adafruit
  ----> https://www.adafruit.com/products/1899

  These displays use I2C to communicate, 2 pins are required to
  interface
 ****************************************************/

#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include <SPI.h>
#include <SD.h>

//const int chipSelect = BUILTIN_SDCARD;

// Connect Vin to 3-5VDC
// Connect GND to ground
// Connect SCL to I2C clock pin (A5 on UNO)
// Connect SDA to I2C data pin (A4 on UNO)
//FILE to w ri t e on the SD ca rd
//File myFile;
Adafruit_HTU21DF htu = Adafruit_HTU21DF();


void setup() {
  
  Serial.begin(9600);
  Serial.println("HTU21D-F test");
 
  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
  //Check for SD card
//  Serial.print(" Initializing SD card... " );
//  if (!SD.begin(chipSelect)){
//    Serial.println(" lol failed ");
//    return ;
//  }
  Serial.println(" initialization done . ");
  //SD.remove(" ABC.txt " ) ; // destroy the file before writing a new one
  //myFile = SD.open (" ABC.txt " , FILE_WRITE) ;
  //myFile.println("Temperature (C)") ;
  //myFile.close() ;

 
  float temp = htu.readTemperature();
  //float rel_hum = htu.readHumidity();
  Serial.print("Temp: "); Serial.print(temp); Serial.print(" C");
  //Serial.print("\t\t");
  //Serial.print("Humidity: "); Serial.print(rel_hum); Serial.println(" \%");


//  myFile = SD.open("toto.txt", FILE_WRITE);
//
//  if (myFile) {
//    Serial.print("Writing toto.txt...");
//    myFile.println("testing 1, 2, 3.");
//    // close the file:
//    myFile.close();
//    Serial.println("done.");
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening test.txt");
//  }


}

void loop() {
    //empty
}
