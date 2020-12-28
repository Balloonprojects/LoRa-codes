#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "file.h"
#include <LoRa.h>

// On définit deux bus SPI différents : le premier pour la carte SD, le second pour le module LoRa

SPIClass spiSD(HSPI);
#define SD_SCK 14
#define SD_MISO 2
#define SD_MOSI 15
#define SD_CS 13
#define SD_Speed 27000000

SPIClass spiLoRa(VSPI);
#define LoRa_SCK 5
#define LoRa_MISO 19
#define LoRa_MOSI 27
#define LoRa_CS 18
#define RST 14
#define DI0 26
#define BAND 436E6

 String LoRaData;


/*  /!\ Pour utiliser le port micro-SD de la carte TTGO LoRa32 OLED v.2, définir les pins du bus SPI tels que: 


 * Le pin n°13 correspond au chipSelect de la carte SD => SD.begin(CS,...).
 * Il correspond en même temps au Slave Select du bus SPI => SPI.begin(SCK,MISO,MOSI,SS)
 *             
 * On a donc SS = CS = 13
 */

void setup(){
    
    Serial.begin(115200);
    
    spiSD.begin(SD_SCK,SD_MISO,SD_MOSI,SD_CS);
    
    if(!SD.begin(SD_CS,spiSD,SD_Speed)){
        Serial.println("Card Mount Failed");
        return;
    }

    spiLoRa.begin(LoRa_SCK,LoRa_MISO,LoRa_MOSI,LoRa_CS);
    LoRa.setPins(LoRa_CS,RST,DI0);

    if(!LoRa.begin(440E6)){
      Serial.println("Starting LoRa failed!");
      while(1);
    }
    
    LoRa.receive();
         
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    writeFile(SD,"/donnees.txt","Header");

    
}
void loop(){
  
  int Size = LoRa.parsePacket();
  if(Size){
    Serial.println("Reçu 5/5");
    LoRaData = LoRa.readString();
    
    LoRaData = LoRaData + "\n";
    
    appendFile(SD,"/donnees.txt",LoRaData);
    
    Serial.println(LoRaData);  
  }
}
