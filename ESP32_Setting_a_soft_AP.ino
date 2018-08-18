#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Preferences.h>
Preferences preferences;

#include "SSD1306Wire.h"

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, 4, 15);

#include "webpage.h"

const int buttonPinSetupAP = 37;    // the number of the pushbutton pin
const int OLED_RSTPin = 16;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 3000;    // the debounce time; increase if the output flickers

int flags_setupAP = 0;
int flags_displayIP = 0;

// Activation Method : OTAA
#define BYTE_devEui 8
#define BYTE_appEui 8
#define BYTE_appKey 16

uint8_t DEVEUI[BYTE_devEui];
uint8_t APPEUI[BYTE_appEui];
uint8_t APPKEY[BYTE_appKey];

// Activation Method : ABP
#define BYTE_devAddr 4
#define BYTE_nwkSKey 16
#define BYTE_appSKey 16

uint32_t DEVADDR;
uint8_t NWKSKEY[BYTE_nwkSKey];
uint8_t APPSKEY[BYTE_appSKey];

#include "ttn.h"
#include "ttn-abp.h"
#include "ttn-otaa.h"

const char *ssid = "MyESP32AP";
const char *password = "testpassword";

AsyncWebServer server(80);

String main_page = MAIN_page;
String abp_page = ABP_page;
String otaa_page = OTAA_page;

String _devAddr;
String _nwkSKey;
String _appSKey;

String _devEui;
String _appEui;
String _appKey; 

void strToHexlittleEndian(String input, uint8_t output[]) {
  int i;
  char upper;
  char lower;
  int indexlittleEndian = (input.length()/2) - 1;
  for (i = 0; i < input.length()/2; i++) {
    
    upper = input.charAt(i * 2);
    lower = input.charAt((i * 2) + 1);

    if(upper >= '0' && upper <= '9') {
      output[indexlittleEndian] = (upper - '0') << 4;
    }
    else if(upper >= 'A' && upper <= 'F') {
      output[indexlittleEndian] = (upper - 0x37) << 4;
    }

    if(lower >= '0' && lower <= '9') {
      output[indexlittleEndian] += (lower - '0');
    }
    else if(lower >= 'A' && lower <= 'F') {
      output[indexlittleEndian] += (lower - 0x37);
    }
    
    indexlittleEndian--;
    
  }
}

void strToHexbigEndian(String input, uint8_t output[]) {
  int i;
  char upper;
  char lower;
  for (i = 0; i < input.length()/2; i++) {
    upper = input.charAt(i * 2);
    lower = input.charAt((i * 2) + 1);

    if(upper >= '0' && upper <= '9') {
      output[i] = (upper - '0') << 4;
    }
    else if(upper >= 'A' && upper <= 'F') {
      output[i] = (upper - 0x37) << 4;
    }

    if(lower >= '0' && lower <= '9') {
      output[i] += (lower - '0');
    }
    else if(lower >= 'A' && lower <= 'F') {
      output[i] += (lower - 0x37);
    }
  }
}

uint32_t strToHex32_t(String input) {
  uint32_t output = 0;
  int i;
  char eachChar;
  uint32_t eachChar32_t;
  int baseNumber = input.length() - 1;
  
  for (i = 0; i < input.length(); i++) {
    Serial.print("baseNumber => ");
    Serial.println(baseNumber);
    eachChar = input.charAt(i);
    Serial.print("eachChar => ");
    Serial.println(eachChar);

    if(eachChar >= '0' && eachChar <= '9') {
      eachChar32_t = (eachChar - '0');
    }
    else if(eachChar >= 'A' && eachChar <= 'F') {
      eachChar32_t = (eachChar - 0x37);
    }
    Serial.print("eachChar32_t => ");
    Serial.println(eachChar32_t, BIN);

    if(baseNumber != 0) {
      Serial.println(eachChar32_t << (4 * baseNumber), BIN);
      output += eachChar32_t << (4 * baseNumber);
    }
    else {
      Serial.println(eachChar32_t, BIN);
      output += eachChar32_t;
    }
    Serial.print("output each => ");
    Serial.println(output, BIN);
    baseNumber--;
  }
  return output;
}

void serialPrintHex(uint8_t arrayValue[], int _length) {
  Serial.print("Size of array : ");
  Serial.println(_length);
  for(int i = 0; i < _length; i++) {
    Serial.print(arrayValue[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPinSetupAP, INPUT);

  preferences.begin("MyESP32", false);

  pinMode(OLED_RSTPin, OUTPUT);
  digitalWrite(OLED_RSTPin, HIGH);
  display.init();

  if(preferences.getString("actMethod", "").equals("abp")) {
    // Activation Method : ABP
    _devAddr = preferences.getString("devAddr", "");
    _nwkSKey = preferences.getString("nwkSKey", "");
    _appSKey = preferences.getString("appSKey", "");

    DEVADDR = strToHex32_t(_devAddr);
    strToHexbigEndian(_nwkSKey, NWKSKEY);
    strToHexbigEndian(_appSKey, APPSKEY);

    serialPrintConfigABP();
    
    setup_ttnABP();
  }
  else {
    // Activation Method : OTAA
    _devEui = preferences.getString("devEui", "");
    _appEui = preferences.getString("appEui", "");
    _appKey = preferences.getString("appKey", "");
    
    strToHexlittleEndian(_devEui, DEVEUI);
    strToHexlittleEndian(_appEui, APPEUI);
    strToHexbigEndian(_appKey, APPKEY);

    serialPrintConfigOTAA();
    
    setup_ttnOTAA();
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", main_page);
  });

  server.on("/activation", HTTP_POST, [](AsyncWebServerRequest *request){
    String buff_actMethod;

    buff_actMethod = request->getParam("method", true)->value();
    preferences.putString("actMethod", buff_actMethod);
    
    if(request->getParam("method", true)->value().equals("abp")){
      request->send(200, "text/html", abp_page);
    }
    else if(request->getParam("method", true)->value().equals("otaa")){
      request->send(200, "text/html", otaa_page);
    }
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
    String buff_devAddr;
    String buff_nwkSKey;
    String buff_appSKey;

    if(request->hasParam("devAddr") || request->hasParam("nwkSKey") || request->hasParam("appSKey")){
      buff_devAddr = request->getParam("devAddr")->value();
      buff_nwkSKey = request->getParam("nwkSKey")->value();
      buff_appSKey = request->getParam("appSKey")->value();
    }
    else {
      
    }
    request->send(200, "text/plain", 
      "Device Address : " + buff_devAddr + "\n" +
      "Network Session Key : " + buff_nwkSKey + "\n" +
      "App Session Key : " + buff_appSKey + "\n"
    );
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
    String buff_devAddr;
    String buff_nwkSKey;
    String buff_appSKey;

    String buff_devEui;
    String buff_appEui;
    String buff_appKey;

    if(request->hasParam("devAddr", true) || request->hasParam("nwkSKey", true) || request->hasParam("appSKey",true)){
      buff_devAddr = request->getParam("devAddr", true)->value();
      buff_nwkSKey = request->getParam("nwkSKey", true)->value();
      buff_appSKey = request->getParam("appSKey", true)->value();

      buff_devAddr.toUpperCase();
      buff_nwkSKey.toUpperCase();
      buff_appSKey.toUpperCase();

      preferences.putString("devAddr", buff_devAddr);
      preferences.putString("nwkSKey", buff_nwkSKey);
      preferences.putString("appSKey", buff_appSKey);
      
      request->send(200, "text/plain", 
        "Device Address : " + buff_devAddr + "\n" +
        "Network Session Key : " + buff_nwkSKey + "\n" +
        "App Session Key : " + buff_appSKey + "\n"
      );

      // Restart ESP
      ESP.restart();
    }
    else if(request->hasParam("devEui", true) || request->hasParam("appEui", true) || request->hasParam("appKey", true)){
      buff_devEui = request->getParam("devEui", true)->value();
      buff_appEui = request->getParam("appEui", true)->value();
      buff_appKey = request->getParam("appKey", true)->value();

      buff_devEui.toUpperCase();
      buff_appEui.toUpperCase();
      buff_appKey.toUpperCase();

      preferences.putString("devEui", buff_devEui);
      preferences.putString("appEui", buff_appEui);
      preferences.putString("appKey", buff_appKey);

      request->send(200, "text/plain", 
        "Device EUI : " + buff_devEui + "\n" +
        "Application EUI : " + buff_appEui + "\n" +
        "App Key : " + buff_appKey + "\n"
      );

      // Restart ESP
      ESP.restart();
    }
    else {
      //
    }
  });
 
  // put your setup code here, to run once:

}

void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPinSetupAP);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        if(!flags_setupAP) {
          WiFi.softAP(ssid, password);
          server.begin();

          Serial.println();
          Serial.print("IP address : ");
          Serial.println(WiFi.softAPIP());
  
          flags_setupAP = 1;
        }
        if(!flags_displayIP) {
          display.drawString(0, 0, WiFi.softAPIP().toString());
          display.display();
          flags_displayIP = 1;
        }
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  os_runloop_once();
  // put your main code here, to run repeatedly:

}

void serialPrintConfigABP() {
  Serial.println("Activation Method : ABP");
  Serial.print("Device Address (Text) : ");
  Serial.println(_devAddr);
  Serial.print("Network Session Key (Text) : ");
  Serial.println(_nwkSKey);
  Serial.print("App Session Key (Text) : ");
  Serial.println(_appSKey);
  
  Serial.println("Device Address (HEX) : ");
  Serial.println(DEVADDR, HEX);
  Serial.println("Network Session Key (HEX) : ");
  serialPrintHex(NWKSKEY, sizeof(NWKSKEY));
  Serial.println("App Session Key (HEX) : ");
  serialPrintHex(APPSKEY, sizeof(APPSKEY));
}

void serialPrintConfigOTAA() {
  Serial.println("Activation Method : OTAA");
  Serial.print("Device EUI (Text) : ");
  Serial.println(_devEui);
  Serial.print("Application EUI (Text) : ");
  Serial.println(_appEui);
  Serial.print("App Key (Text) : ");
  Serial.println(_appKey);
  
  Serial.println("Device EUI (HEX) : ");
  serialPrintHex(DEVEUI, sizeof(DEVEUI));
  Serial.println("Application EUI (HEX) : ");
  serialPrintHex(APPEUI, sizeof(APPEUI));
  Serial.println("App Key (HEX) : ");
  serialPrintHex(APPKEY, sizeof(APPKEY));
}

