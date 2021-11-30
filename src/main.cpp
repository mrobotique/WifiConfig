/*
WiFi manager tuto https://www.mischianti.org/2020/10/11/how-to-manage-dynamic-wifi-configuration-on-esp8266-or-esp32/
*/

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>         //Mqtt PUB-SUB Client
#include <EEPROM.h>

// select which pin will trigger the configuration portal when set to LOW
// ESP-01 users please note: the only pins available (0 and 2), are shared 
// with the bootloader, so always set them HIGH at power-up
#define TRIGGER_PIN D5

char mqtt_server[40];
char mqtt_port[6];
char device_id[7];

//EEPROM.write(addr, val);
const int EEPROM_IP_Addr = 0;
const int EEPROM_PORT_Addr = 41;
const int EEPROM_DEVICE_ID_Addr = 48;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\n Starting");

  pinMode(TRIGGER_PIN, INPUT_PULLUP);
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset, int str_len)
{
  char data[str_len + 1];
  for (int i = 0; i < str_len; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[str_len] = '\0'; 
  Serial.println(data);
  return String(data);
}

void readParamsFromEEPROM()
{
  Serial.print("---> ########## From eeprom: ");
  Serial.println(readStringFromEEPROM(EEPROM_IP_Addr, 40));
}


WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
WiFiManagerParameter custom_device_id("device_id", "device id", device_id, 6);
WiFiManager wifiManager;

void loop() {
  // is configuration portal requested?
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
      wifiManager.addParameter(&custom_mqtt_server);
      wifiManager.addParameter(&custom_mqtt_port);
      wifiManager.addParameter(&custom_device_id);
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    

    //reset settings - for testing
    //wifiManager.resetSettings();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
    //WiFi.mode(WIFI_STA);
    
    if (!wifiManager.startConfigPortal("OnDemandAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP
    Serial.println("***** Mqtt Data ****");
    Serial.println(custom_mqtt_server.getValue());
    Serial.println(custom_mqtt_port.getValue());
    Serial.println(custom_device_id.getValue());

    //Save 
    writeStringToEEPROM(EEPROM_IP_Addr, (char*)"---------------------------------------");
    writeStringToEEPROM(EEPROM_IP_Addr,(char*)custom_mqtt_server.getValue());

    writeStringToEEPROM(EEPROM_PORT_Addr, (char*)"-----");
    writeStringToEEPROM(EEPROM_PORT_Addr,(char*) custom_mqtt_port.getValue());

    writeStringToEEPROM(EEPROM_DEVICE_ID_Addr, (char*)"------");
    writeStringToEEPROM(EEPROM_DEVICE_ID_Addr, (char*)custom_device_id.getValue());




    Serial.println("***********************************");
  }

  

  // put your main code here, to run repeatedly:
  if (!WiFi.isConnected()){
    wifiManager.autoConnect();
    Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP
    
  }
  
  
}