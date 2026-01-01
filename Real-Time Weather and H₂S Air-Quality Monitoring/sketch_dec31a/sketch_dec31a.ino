#include <WiFi.h>
#include "ThingSpeak.h"
#include "DHT.h"

// WiFi Credentials
const char* ssid = "AC/DC";      
const char* password = "#UseFreeWifiHere"; 

// ThingSpeak Details
unsigned long myChannelNumber = 3202581;    
const char * myWriteAPIKey = "7GLS2SPONGAPKSM1"; 

// Sensor Pins
#define DHTPIN 4
#define DHTTYPE DHT22
#define MQ7_AO 34
#define MQ136_AO 35  // Added MQ136 on GPIO 35

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Important for ESP32 to read higher voltages (up to 3.3V)
  analogSetAttenuation(ADC_11db);
  
  // WiFi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  
  ThingSpeak.begin(client); 
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int mq7Value = analogRead(MQ7_AO);
  int mq136Value = analogRead(MQ136_AO); // Read MQ136

  // Check if DHT readings are valid
  if (isnan(h) || isnan(t)) {
    Serial.println("DHT sensor read error!");
    // We don't 'return' here so that MQ sensors can still upload even if DHT fails
  }

  // Print to Serial Monitor for debugging
  Serial.print("Temp: "); Serial.print(t);
  Serial.print(" | Hum: "); Serial.print(h);
  Serial.print(" | MQ7: "); Serial.print(mq7Value);
  Serial.print(" | MQ136: "); Serial.println(mq136Value);

  // Set ThingSpeak Fields
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, mq7Value);
  ThingSpeak.setField(4, mq136Value); // MQ136 data on Field 4

  // Upload Data
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(x == 200){
    Serial.println("ThingSpeak update successful.");
  } else {
    Serial.println("Update error. HTTP code: " + String(x));
  }

  // Wait 20 seconds for the next update
  delay(20000); 
}