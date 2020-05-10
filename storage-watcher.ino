#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <WEMOS_SHT3X.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

#include "config.h"

const char* REST_API_END_POINT = "https://api.thingspeak.com";

const int TIMEOUT = 3600000;
const long DEEP_SLEEP_TIMEOUT = 1.8e9;

/* temperature and humidity sensor */
SHT3X sht30(0x45);

WiFiClient client;


void displaySensorDetails(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  ThingSpeak.begin(client); 
  Serial.println("ThingSpeak client initialized");
}

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("SHT30 storage watcher"); 
  Serial.println("");
}

void loop(void) {
  Serial.println("<<loop iteration>>");
  if(sht30.get()==0) {
      Serial.println("sht30 reading ok");
      String sTemp = String(sht30.cTemp);
      String sHum = String(sht30.humidity);
      Serial.print("Temperature [C]: ");
      Serial.println(sTemp);
      Serial.print("Relative Humidity [%]: ");
      Serial.println(sHum);
      ThingSpeak.setField(1, sTemp);
      ThingSpeak.setField(2, sHum);

      int HTTP_RETURN_CODE = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      // Check the return code
      if(HTTP_RETURN_CODE == 200){
        Serial.println("Channel update successful.");
      }
      else{
        Serial.println("Problem updating channel. HTTP error code " + String(HTTP_RETURN_CODE));
      }
  } else {
    Serial.println("SHT30 not found!");
  }
  Serial.println("<<~iteration>>");
  ESP.deepSleep(DEEP_SLEEP_TIMEOUT);//twice-an hour total
  ESP.deepSleep(DEEP_SLEEP_TIMEOUT);
}
