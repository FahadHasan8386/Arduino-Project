#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// ================= WiFi =================
const char* ssid     = "AC/DC";
const char* password = "#UseFreeWifiHere";

// ================= API URLs =================
const char* DHT_URL   = "http://192.168.1.3:9191/api/DHTSensor/CreateDHTData";
const char* MQ7_URL   = "http://192.168.1.3:9191/api/MQ7Sensor/CreateMQ7Data";
const char* MQ136_URL = "http://192.168.1.3:9191/api/MQ136Sensor/CreateMQ136Data";

// ================= Pins =================
#define DHTPIN    4
#define DHTTYPE  DHT22
#define MQ7_PIN   34
#define MQ136_PIN 35

DHT dht(DHTPIN, DHTTYPE);

// ================= Timing =================
unsigned long lastTime = 0;
const unsigned long timerDelay = 10000; // 10 sec

// ================= HTTP POST =================
void postData(const char* url, const String& jsonPayload)
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected");
    return;
  }

  WiFiClient client;          // 🔥 HTTP client
  HTTPClient http;

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonPayload);

  Serial.println("--------------------------------");
  Serial.print("POST URL: ");
  Serial.println(url);
  Serial.print("Payload: ");
  Serial.println(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("✅ Response Code: ");
    Serial.println(httpResponseCode);
    Serial.println("Response Body:");
    Serial.println(http.getString());
  } else {
    Serial.print("❌ POST Error Code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// ================= Setup =================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ================= Loop =================
void loop()
{
  if (millis() - lastTime >= timerDelay)
  {
    // -------- DHT22 --------
    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();

    if (!isnan(temperature) && !isnan(humidity))
    {
      String dhtPayload =
        "{\"Temperature\":" + String(temperature, 2) +
        ",\"Humidity\":" + String(humidity, 2) + "}";

      postData(DHT_URL, dhtPayload);
    }
    else
    {
      Serial.println("❌ DHT22 read failed");
    }

    // -------- MQ7 --------
    int mq7Value = analogRead(MQ7_PIN);
    String mq7Payload = "{\"COLevel\":" + String(mq7Value) + "}";
    postData(MQ7_URL, mq7Payload);

    // -------- MQ136 --------
    int mq136Value = analogRead(MQ136_PIN);
    String mq136Payload = "{\"H2SLevel\":" + String(mq136Value) + "}";
    postData(MQ136_URL, mq136Payload);

    lastTime = millis();
  }
}