#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "kSeries.h"

#define SEALEVELPRESSURE_HPA (1013.25)
 
Adafruit_BME680 bme;
kSeries K_30(12,13);

char ssid[] = "Kiprovi2";
char password[] = "azsumbog123";

const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "1VRODSOH2BWSPIGM";

unsigned long lastTime = 0;
unsigned long timerDelay = 1800000; //30 минути

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Every 30 minutes");
  
  if (!bme.begin(0x77)) 
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); 
}

void loop() {
   double co2 = K_30.getCO2('p');
   if (! bme.performReading()) 
  {
    Serial.println("Failed to perform reading :(");
    return;
  }
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      http.begin(serverName);
      Serial.print("Temperature = ");
      Serial.print(bme.temperature);
      Serial.println(" *C");
 
      Serial.print("Pressure = ");
      Serial.print(bme.pressure / 100.0);
      Serial.println(" hPa");
      
      Serial.print("Humidity = ");
      Serial.print(bme.humidity);
      Serial.println(" %");
      
      Serial.print("Approx. Altitude = ");
      Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
      Serial.println(" m");
      
      Serial.print("Gas = ");
      Serial.print(bme.gas_resistance / 1000.0);
      Serial.println(" KOhms");

      Serial.print("Co2 ppm = ");
      Serial.println(co2); 
 
      
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
 
      String httpRequestData = "api_key=" + apiKey + "&field1=" + String(bme.temperature) + "&field2=" + String(bme.pressure / 100.0) + "&field3=" + String(bme.humidity)+ "&field5=" + String(bme.gas_resistance / 1000.0)+"&field6=" + String(co2); 
        
      
      int httpResponseCode = http.POST(httpRequestData);
    
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
