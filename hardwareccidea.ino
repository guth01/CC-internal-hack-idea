#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "1+";
const char* password = "pistachio";

AsyncWebServer server(80);

int fanPin = 16;
int dutyCycle = 0;

float temp = 0;
float humidity = 0;
int threshold = 30;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(fanPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  Temperature ");
  lcd.setCursor(0, 1);
  lcd.print("Monitoring System");
  delay(4000);
  lcd.clear();

  analogWriteRange(100);
  analogWriteFreq(10000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"temperature\":";
    json += String(temp);
    json += ", \"humidity\":";
    json += String(humidity);
    json += "}";
    request->send(200, "application/json", json);
  });

  server.on("/set_threshold", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("threshold", true)) {
      String thresholdValue = request->getParam("threshold", true)->value();
      threshold = thresholdValue.toInt();
      Serial.print("Threshold set to: ");
      Serial.println(threshold);
    }
    request->send(200, "text/plain", "Threshold updated");
  });

  server.begin();
}

void controlFanSpeed(int fanSpeedPercent) {
  analogWrite(fanPin, fanSpeedPercent);

  Serial.print("Fan Speed: ");
  Serial.print(fanSpeedPercent);
  Serial.println("%");

  lcd.setCursor(0, 1);
  lcd.print("Fan Speed: ");
  lcd.print(fanSpeedPercent);
  lcd.print("%");
}

void loop() {
  temp = dht.readTemperature();
  humidity = dht.readHumidity();

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("*C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print("*C ");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");

  if (temp >= threshold) {
    int fanSpeedPercent = map(temp, threshold, 55, 10, 100);
    controlFanSpeed(fanSpeedPercent);
  } else {
    controlFanSpeed(0);
  }
}

