// will play many audio file formats, mp3,aac,flac etc.
// See github page : https://github.com/schreibfaul1/ESP32-audioI2S
#include <WebServer.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 433E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Digital I/O used SD CARD
#define SD_CS         21
#define SPI_MOSI      12   // SD Card
#define SPI_MISO      33
#define SPI_SCK       13
//Audio
#define I2S_DOUT      38
#define I2S_BCLK      37   // I2S
#define I2S_LRC       39

//packet counter
int counter = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

volatile bool beeson = false;
Audio audio;


String LoRaData;

void setup() {
    Serial.begin(115200);

    //reset OLED display via software
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);
  
    //initialize OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }
    
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("BEEKEEPA.. ");
    display.display();
  
    
    //SPI LoRa pins
    SPI.begin(SCK, MISO, MOSI,SS);
    //setup LoRa transceiver module
    LoRa.setPins(SS, RST, DIO0);    
    if (!LoRa.begin(BAND)) {
      Serial.println("Starting LoRa failed!");
      while (1);
    }
    Serial.println("LoRa Initializing OK!");
    display.setCursor(0,10);
    display.print("LoRa Buzzing!");
    display.display();
    delay(2000);
      
//   
    pinMode(SD_CS, OUTPUT);      
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);   
    if(!SD.begin(SD_CS))
    {
      Serial.println("Error talking to SD card!");
      while(true);  // end program
    }
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0...21
    audio.connecttoFS(SD,"/beehive.mp3");
    audio.setFileLoop(true);
}

void loop()
{
   //try to parse packet
  int packetSize = LoRa.parsePacket();
    if (packetSize) {
      //received a packet
      Serial.print("Received packet ");
  
      //read packet
      while (LoRa.available()) {
        LoRaData = LoRa.readString();
        Serial.print(LoRaData);
      }
  
      //print RSSI of packet
      int rssi = LoRa.packetRssi();
      Serial.print(" with RSSI ");    
      Serial.println(rssi);
  
     // Dsiplay information
     display.clearDisplay();
     display.setCursor(0,0);
     display.print("BEEKEEPA..");
     display.setCursor(0,20);
     display.print("Received packet:");
     display.setCursor(0,30);
     display.print(LoRaData);
     display.setCursor(0,40);
     display.print("RSSI:");
     display.setCursor(30,40);
     display.print(rssi);
     display.display();   
     if (LoRaData == "hello 2"){
        Serial.println("Bees on");
        display.setCursor(0,40);
        display.print("Bees!");
        display.setCursor(0,50);
        beeson = true;
       
        
      } 
      if(beeson == true){
      
        audio.loop();    
      }
    }
}
