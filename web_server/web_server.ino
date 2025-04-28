#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DHT.h>

// Pin Definitions
#define DHTPIN 2      
#define DHTTYPE DHT11
#define SOIL_SENSOR 34
#define AQI_SENSOR 35

// WiFi Credentials
const char* ssid = "retenet";
const char* password = "qwertyuiopp";

// Create Server and Sensor Objects
AsyncWebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

// Sensor Data Variables
String temperature = "N/A";
String humidity = "N/A";
String soilMoisture = "N/A";
String airQuality = "N/A";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();

  // Wait for WiFi Connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi: " + String(WiFi.localIP()));

  // Define Web Server Route
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><title>Soil & Air Monitoring</title></head><body>";
    html += "<h1>Environmental Data</h1>";
    html += "<p><strong>Temperature:</strong> " + temperature + " °C</p>";
    html += "<p><strong>Humidity:</strong> " + humidity + " %</p>";
    html += "<p><strong>Soil Moisture:</strong> " + soilMoisture + "</p>";
    html += "<p><strong>Air Quality (AQI Sensor):</strong> " + airQuality + "</p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Start Server
  server.begin();
}

void loop() {
  // Read Sensors
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soilMoist = analogRead(SOIL_SENSOR);
  int aqiVal = analogRead(AQI_SENSOR);

  // Handle Read Failures
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = "Error";
    humidity = "Error";
  } else {
    temperature = String(temp, 1);  // 1 decimal place
    humidity = String(hum, 1);
  }

  soilMoisture = String(soilMoist);
  airQuality = String(aqiVal);

  // Debug Output
  Serial.println("=== Sensor Readings ===");
  Serial.println("Temperature: " + temperature + " °C");
  Serial.println("Humidity: " + humidity + " %");
  Serial.println("Soil Moisture: " + soilMoisture);
  Serial.println("Air Quality: " + airQuality);
  Serial.println("========================");

  delay(2000);
}