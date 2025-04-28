#include <DHT.h>

#define DHTPIN 2      // Pin where the DHT11 is connected
#define DHTTYPE DHT11
#define SOIL_SENSOR A0 // Analog pin for soil moisture sensor
#define AQI_SENSOR A1  // Analog pin for AQI sensor (MQ135)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soilMoisture = analogRead(SOIL_SENSOR);
  int aqiValue = analogRead(AQI_SENSOR); 

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.print(", AQI Value: ");
  Serial.println(aqiValue);

  delay(2000); 
}
