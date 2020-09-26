#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <WEMOS_SHT3X.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <DS18B20.h>
#include "Adafruit_SGP30.h"

#include "config.h"

const char* REST_API_END_POINT = "https://api.thingspeak.com";

const int TIMEOUT = 600000;//10min
//1hr//3600000;
//const long DEEP_SLEEP_TIMEOUT = 1.8e9; //30min
const long DEEP_SLEEP_TIMEOUT = 6.0e8; //10min

/* sensor equipment */
/* temperature probe */
DS18B20 ds(D4);
uint8_t address[] = {40, 250, 31, 218, 4, 0, 0, 52};
uint8_t selected;

/* temperature and humidity sensor */
SHT3X sht30(0x45);

/* air quality sensor */
Adafruit_SGP30 sgp;

WiFiClient client;

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("SHT30 storage watcher"); 
  Serial.println("");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("DS Devices: ");
  Serial.println(ds.getNumberOfDevices());
  Serial.println();
  Serial.println("");
  if (! sgp.begin()){
    Serial.println("SGP Sensor not found :(");
    delay(500);
    while (1);
  }
  Serial.println("WiFi connected");
  ThingSpeak.begin(client); 
  Serial.println("ThingSpeak client initialized");
}

void loop(void) {
  Serial.println("<<loop iteration>>");
  float temperature = -1000;
  float humidity = -1000;
  String sAirTemp = "";
  String sAirHum = "";
  String sTVOC = "";
  String seCO2 = "";
  String sFluidTemp = "";
  
  if(sht30.get()==0) {
      Serial.println("sht30 reading ok");
      temperature = sht30.cTemp;
      humidity = sht30.humidity;
      sAirTemp = String(temperature);
      sAirHum = String(humidity);
      Serial.print("Air Temperature [C]: ");
      Serial.println(sAirTemp);
      Serial.print("Air Relative Humidity [%]: ");
      Serial.println(sAirHum);

      Serial.print("Fluid Temperature [C]: ");
      sFluidTemp = String(ds.getTempC());
      Serial.println(sFluidTemp);
  } else {
    Serial.println("SHT30 not found!");
  }
  if (temperature >-1000 && humidity >-1000) {
    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
  }
  if (! sgp.IAQmeasure()) {
    Serial.println("SGP30 Measurement failed");
  } else {
    sTVOC = sgp.TVOC;
    seCO2 = sgp.eCO2;
    Serial.print("TVOC "); 
    Serial.print(sTVOC); 
    Serial.print(" ppb\t");
    Serial.print("eCO2 "); 
    Serial.print(seCO2); 
    Serial.println(" ppm");
  }
  Serial.println("<<~iteration>>");
  ThingSpeak.setField(1, sAirTemp);
  ThingSpeak.setField(2, sAirHum);
  ThingSpeak.setField(4, sFluidTemp);
  ThingSpeak.setField(5, sTVOC);
  ThingSpeak.setField(6, seCO2);

  int HTTP_RETURN_CODE = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  // Check the return code
  if(HTTP_RETURN_CODE == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(HTTP_RETURN_CODE));
  }

  //ESP.deepSleep(DEEP_SLEEP_TIMEOUT);//twice-an hour total
  ESP.deepSleep(DEEP_SLEEP_TIMEOUT);
}

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}
