// will play many audio file formats, mp3,aac,flac etc.
// See github page : https://github.com/schreibfaul1/ESP32-audioI2S
#include <WiFi.h>
#include <WebServer.h>
#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"


// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23    // SD Card
#define SPI_MISO      19
#define SPI_SCK       18

#define I2S_DOUT      25
#define I2S_BCLK      27    // I2S
#define I2S_LRC       26

volatile bool beeson = false;
Audio audio;


/* Put your SSID & Password */
const char* ssid = "BEES";  // Enter SSID here
const char* password = "honeybees";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,0,10);
IPAddress gateway(192,168,0,10);
IPAddress subnet(255,255,255,0);

WebServer server(80);

void setup() {
    //Setup esp as access point
  
    Serial.begin(115200);
    WiFi.mode(WIFI_AP);
    Serial.print("Setting soft-AP ... ");
    WiFi.softAP(ssid, password);
    delay(100);
    Serial.print("Setting soft-AP configuration ... ");
    Serial.println(WiFi.softAPConfig(local_ip, gateway, subnet) ? "Ready" : "Failed!");
        
  
  
    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());
    delay(100);
      //setup gate timing and io pins    
    server.on("/", handle_OnConnect);
    server.on("/bees", handle_beeson);
    server.on("/nobees", handle_beesoff);
    server.onNotFound(handle_NotFound);
    server.begin();
    
    pinMode(SD_CS, OUTPUT);      
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    Serial.begin(115200);
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
    server.handleClient();
    if (beeson == true){
       Serial.println("Bees on");
      audio.loop();    
      
    } 
}

void handle_OnConnect() { 
  
  server.send(200, "text/html", SendHTML(false));
}


void handle_beeson() {
  beeson = true;
  server.send(200, "text/html", SendHTML(true));
}

void handle_beesoff() {
  beeson = false;
  server.send(200, "text/html", SendHTML(false));
}


void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Elephant Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "* {  box-sizing: border-box;}\n";
  ptr += "body {  margin: 0;  padding: 0; justify-content: center;  align-items: center;  min-height: 100vh;  background: #999;} h1 {color: #5076e0;margin: 50px auto 30px;} h3 {color: #ffffff;margin-bottom: 50px;}\n";
  ptr += ".button-on {width: 150px; height: 150px;  border: 3px outset #888;  position: relative; display: inline-block;  padding: 50px 30px; color: #eee;  text-transform: uppercase;  letter- overflow: hidden; box-shadow: 0 0 10px rgb(0, 0, 0, 1); font-family: verdana; font-size: 28px;  font-weight: bolder;  text-decoration: none;  background:linear-gradient(160deg, #666, #444); text-shadow: 0px 0px 2px rgba(0, 0, 0, .5); transition: 0.2s;border-radius: 50%;}\n";
  ptr += ".button-off {width: 150px; height: 150px;  border: 3px outset #888;  position: relative; display: inline-block;  padding: 50px 30px; color: #eee;  text-transform: uppercase;  letter- overflow: hidden; box-shadow: 0 0 10px rgb(1, 0, 0, 0); font-family: verdana; font-size: 28px;  font-weight: bolder;  text-decoration: none;  background:linear-gradient(160deg, #666, #444); text-shadow: 0px 0px 2px rgba(0, 0, 0, .5); transition: 0.2s;border-radius: 50%;}\n";
 
  ptr += "a:active {  border: 3px outset #ddd;  color: #fff;  background: linear-gradient(160deg, #666, #444);  text-shadow: 0px 0px 4px #ccc;  box-shadow: 0 0 10px #fff, 0 0 40px #fff, 0 0 80px #fff;  transition-delay: 1s;}\n";
  ptr += "a span {: block;}\n";
  ptr += "a span:nth-child(1) {  top: 0; left: -100%;  width: 100%;  height: 2px;  background: linear-gradient(90deg, transparent, #eee);}\n";
  ptr += "a:active span:nth-child(1) {  left: 100%; transition: 1s;}\n";
  ptr += "a span:nth-child(2) {  top: -100%; right: 0; width: 2px; height: 100%; background: linear-gradient(180deg, transparent, #eee);}\n";
  ptr += "a:active span:nth-child(2) {  top: 100%;  transition: 1s; transition-delay: 0.25s;}\n";
  ptr += "a span:nth-child(3) {  bottom: 0;  right: -100%; width: 100%;  height: 2px;  background: linear-gradient(270deg, transparent, #eee);}\n";
  ptr += "a:active span:nth-child(3) {  right: 100%;  transition: 1s; transition-delay: 0.5s;}\n";
  ptr += "a span:nth-child(4) {  bottom: -100%;  left: 0;  width: 2px; height: 100%; background: linear-gradient(360deg, transparent, #eee);}\n";
  ptr += "a:active span:nth-child(4) {  bottom: 100%; transition: 1s; transition-delay: 0.75s;}\n";  
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>BeeKeepa</h1>\n";
  ptr +="<h3>WiFi Remote</h3>\n";
 
  if(led1stat)
  {ptr +="<p>Bee Status: ON</p><a class=\"button button-off\" href=\"/nobees\">PUSH<span></span><span></span><span></span><span></span><span></span></a>\n";}
  else
  {ptr +="<p>Bee Status: OFF</p><a class=\"button button-on\" href=\"/bees\">PUSH<span></span><span></span><span></span><span></span><span></span></a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
