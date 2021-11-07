
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Audio.h"
#include "FS.h"

// SD SPI pins can be chosen freely, but cannot overlap with other ports!
#define SD_CS 23
#define SD_SCK 17
#define SD_MOSI 12
#define SD_MISO 21

#define LORA_FREQ 433.0

#define LOG_PATH "/lora_recv.log"

// The sd card can also use a virtual SPI bus
SPIClass sd_spi(HSPI);

// Use a virtual (software) SPI bus for the sx1278
RHSoftwareSPI sx1278_spi;
RH_RF95 rf95(LORA_CS, LORA_IRQ, sx1278_spi);

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//AUDIO
#define I2S_DOUT      25
#define I2S_BCLK      33    // I2S
#define I2S_LRC       32
#define AUDIO_EN      02
volatile bool beeson = false;
Audio audio;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
    
    // Serial output
    Serial.begin(115200);
    pinMode(SD_CS, OUTPUT);
    pinMode(LORA_CS, OUTPUT);
    pinMode(AUDIO_EN, OUTPUT);
    digitalWrite(AUDIO_EN, LOW);
    digitalWrite(LORA_CS, HIGH);
    digitalWrite(SD_CS, HIGH);  
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

  
    // Builtin LED

    // LoRa: Init
    pinMode(LORA_RST, OUTPUT);
    digitalWrite(LORA_RST, LOW);
    delay(100);
    digitalWrite(LORA_RST, HIGH);

    // the pins for the virtual SPI explicitly to the internal connection
    sx1278_spi.setPins(LORA_MISO, LORA_MOSI, LORA_SCK);

    if (!rf95.init()) 
        Serial.println("LoRa Radio: init failed.");
    else
        Serial.println("LoRa Radio: init OK!");

    // LoRa: set frequency
    if (!rf95.setFrequency(LORA_FREQ))
        Serial.println("LoRa Radio: setFrequency failed.");
    else
        Serial.printf("LoRa Radio: freqency set to %f MHz\n", LORA_FREQ);

    rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128);

    // LoRa: Set max (23 dbm) transmission power. 
    rf95.setTxPower(23, false);
    delay(4000);
    display.clearDisplay();
    display.setCursor(0,10);
    display.print("Checking SD CARD");
    display.display();  
    delay(1000);

        pinMode(LED_BUILTIN, OUTPUT);
    String lorapins = "CS:";
    lorapins.concat(LORA_CS);
    lorapins.concat("MISO:");
    lorapins.concat(LORA_MISO);
    lorapins.concat("MOSI:");
    lorapins.concat(LORA_MOSI);
    lorapins.concat("SCK:");
    lorapins.concat(LORA_SCK);
    lorapins.concat("IRQ:");
    lorapins.concat(LORA_IRQ);
    Serial.println(lorapins);
    
    // SD Card
    sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    display.clearDisplay();
    if (!SD.begin(SD_CS, sd_spi)){
      
        display.setCursor(0,10);
        display.print("SD Card: mounting failed.");
        Serial.println("SD Card: mounting failed.");
             display.display();  
    }
    else {
        display.setCursor(0,10);
        display.print("SD Card: mounted.");
        Serial.println("SD Card: mounted.");
             display.display();  
    }
    
   //AUDIO 
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // 0...21
  audio.connecttoFS(SD,"/beehive.mp3");
  audio.setFileLoop(true);
  display.clearDisplay();
  display.setCursor(0,10);
  display.print("Audio Configured");
  Serial.println("Audio Configured");
  display.display();  
}

uint8_t lora_buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t lora_len;
uint8_t receive_counter = 0;

void loop() {
    // Blink LED
    //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    lora_len = RH_RF95_MAX_MESSAGE_LEN;
    if (rf95.recv(lora_buf, &lora_len)) {
        receive_counter++;
        Serial.printf("Received LoRa message #%i (%i bytes):\n%s\n", receive_counter, lora_len, lora_buf);
         display.clearDisplay();
         display.setCursor(0,0);
         display.print("BEEKEEPA..");
         display.setCursor(0,20);
         display.print("Received packet");
         display.setCursor(0,30);
         display.printf("%s", lora_buf);
         display.display(); 
           
         String dataReceived = (char*)lora_buf;
          if(dataReceived == "Hello World5"){
            beeson = true;
          }
          if(dataReceived == "Hello World10"){
            beeson = false;
          }

        // Writing to file works but disabled when playing audio 
//        File test = SD.open(LOG_PATH, FILE_APPEND);
//        if (!test) {
//            Serial.println("SD Card: writing file failed.");
//        } else {
//            Serial.printf("SD Card: appending data to %s.\n", LOG_PATH);
//            test.write(lora_buf, lora_len);
//            test.printf("\n\n");++
//            test.close();
//        }

          
    }
    if (beeson == true){
        Serial.println("Bees on");
        digitalWrite(AUDIO_EN, HIGH);
        audio.loop();                
    }else {
        digitalWrite(AUDIO_EN, LOW);
    }

    //delay(100);
}
