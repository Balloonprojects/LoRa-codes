#include <SPI.h>
#include <SD.h>



File myFile = SD.open("toto.txt");
// Gestion erreur ici

Serial.print(F("Taille toto.txt : "));
Serial.print(fichier.size());
Serial.println(F(" octets"));
