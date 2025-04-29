#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define SOIL_SENSOR 34

const char* ssid = "retenet";
const char* password = "qwertyuiopp";

AsyncWebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

String temperature = "N/A";
String humidity = "N/A";
String soilMoisture = "N/A";

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html><head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>ESP32 Sensor Dashboard</title>
      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
      <style>
        body { font-family: Arial; background: #eef2f3; text-align: center; }
        nav { background: #333; padding: 1em; }
        nav a { color: white; margin: 10px; text-decoration: none; font-size: 20px; }
        .card { background: white; margin: 20px auto; padding: 20px; border-radius: 10px; width: 90%; box-shadow: 0 0 10px rgba(0,0,0,0.2); }
      </style>
      </head><body>
      <nav>
        <a href="/">Dashboard</a>
      </nav>

      <div class="card">
        <h2>Sensor Readings</h2>
        <p><strong>Temperature:</strong> <span id="temperature">--</span> °C</p>
        <p><strong>Humidity:</strong> <span id="humidity">--</span> %</p>
        <p><strong>Soil Moisture:</strong> <span id="soilMoisture">--</span></p>
        <h3 style="color: green;">Prediction: <span id="prediction">Calculating...</span></h3>
        <canvas id="tempChart"></canvas>
        <canvas id="soilChart"></canvas>
      </div>

      <script>
        let tempData = [];
        let soilData = [];
        let labels = [];

        function predictIrrigation(temp, humidity, soil) {
          if (soil < 1500) {
            return "Water needed: Soil is too dry!";
          } else if (humidity < 40 && temp > 30) {
            return "Caution: High temperature and low humidity, consider watering!";
          } else {
            return "No immediate watering needed.";
          }
        }

        function fetchData() {
          fetch('/data').then(resp => resp.json()).then(data => {
            document.getElementById('temperature').textContent = data.temperature;
            document.getElementById('humidity').textContent = data.humidity;
            document.getElementById('soilMoisture').textContent = data.soilMoisture;
            
            const predictionText = predictIrrigation(parseFloat(data.temperature), parseFloat(data.humidity), parseInt(data.soilMoisture));
            document.getElementById('prediction').textContent = predictionText;

            if (labels.length > 20) {
              labels.shift();
              tempData.shift();
              soilData.shift();
            }

            const now = new Date().toLocaleTimeString();
            labels.push(now);
            tempData.push(parseFloat(data.temperature));
            soilData.push(parseInt(data.soilMoisture));

            tempChart.update();
            soilChart.update();
          });
        }

        setInterval(fetchData, 2000);

        const tempChart = new Chart(document.getElementById('tempChart').getContext('2d'), {
          type: 'line',
          data: {
            labels: labels,
            datasets: [{
              label: 'Temperature (°C)',
              data: tempData,
              borderColor: 'red',
              fill: false
            }]
          }
        });

        const soilChart = new Chart(document.getElementById('soilChart').getContext('2d'), {
          type: 'line',
          data: {
            labels: labels,
            datasets: [{
              label: 'Soil Moisture',
              data: soilData,
              borderColor: 'blue',
              fill: false
            }]
          }
        });

      </script>
      </body></html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<200> jsonDoc;

    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["soilMoisture"] = soilMoisture;

    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);
    
    request->send(200, "application/json", jsonResponse);
  });

  server.begin();
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soil = analogRead(SOIL_SENSOR);

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    temperature = "Error";
    humidity = "Error";
  } else {
    temperature = String(temp, 1);
    humidity = String(hum, 1);
  }

  soilMoisture = String(soil);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Soil Moisture: ");
  Serial.println(soilMoisture);

  delay(2000);
}
