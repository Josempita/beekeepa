
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



#define LORA_FREQ 433.0

#define LOG_PATH "/lora_recv.log"


// Use a virtual (software) SPI bus for the sx1278
RHSoftwareSPI sx1278_spi;
RH_RF95 rf95(LORA_CS, LORA_IRQ, sx1278_spi);

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Button definitions
#define BUTTON_PIN  32
#define DEBOUNCE_TIME  50 

int lastSteadyState = LOW;       // the previous steady state from the input pin
int lastFlickerableState = LOW;  // the previous flickerable state from the input pin
int currentState;                // the current reading from the input pin

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);



void setup() {
    //Button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
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
    display.print("BEEKEEPA.SENDER ");
    display.display();

  
    // Builtin LED
    pinMode(LED_BUILTIN, OUTPUT);

    // Serial output
    Serial.begin(115200);

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

    

}

uint8_t lora_buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t lora_len;
uint8_t send_counter = 0;
String  hello = "Hello World";
void loop() {

  currentState = digitalRead(BUTTON_PIN);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch/button changed, due to noise or pressing:
  if (currentState != lastFlickerableState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    // save the the last flickerable state
    lastFlickerableState = currentState;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(lastSteadyState == HIGH && currentState == LOW)
      sendState(true);
    else if(lastSteadyState == LOW && currentState == HIGH)
      sendState(false);

    // save the the last steady state
    lastSteadyState = currentState;
  }

  

}

void sendState(bool isOn){
  String message = "";
  if(isOn == true){
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
    message = hello + "5";
  } else {
    message = hello + "10";
  }
  
  uint8_t data[message.length()+1];
  message.getBytes(data, message.length()+1);
  rf95.send(data, sizeof(data));
    
  rf95.waitPacketSent();  
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("BEEKEEPA..");
  display.setCursor(0,20);
  display.print("Sent packet");
  display.setCursor(0,30);
  display.print(message);
  display.display();    
  delay(100);  
}
