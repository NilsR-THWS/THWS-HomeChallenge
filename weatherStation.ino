#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SDA_PIN 8
#define SCL_PIN 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "WIFI-NAME";
const char* password = "WIFI-PASSWORD";

// -----------------------------------------
// OPEN-METEO API URL
// München: 48.137 → 11.575
// -----------------------------------------
String weatherURL =
  "https://api.open-meteo.com/v1/forecast?"
  "latitude=48.137&longitude=11.575&current_weather=true";

// -----------------------------------------
void drawWeatherIcon(int code, int x, int y) {

  if (code == 0) {
    // Klar
    display.setCursor(x, y);
    display.print("O");
  }
  else if (code == 1 || code == 2 || code == 3) {
    // Wolkig
    display.setCursor(x, y);
    display.print("( )");
  }
  else if (code >= 37 && code <= 49) {
    // Wolken/Nebel
    display.setCursor(x, y);
    display.print("///");
  }
  else if (code >= 50 && code <= 67) {
    // Regen
    display.setCursor(x, y);
    display.print("~~");
  }
  else if (code >= 71 && code <= 77) {
    // Schnee
    display.setCursor(x, y);
    display.print("***");
  }
  else if (code >= 80 && code <= 82) {
    // Schauer
    display.setCursor(x, y);
    display.print("///");
  }
  else {
    // Unbekannt
    display.setCursor(x, y);
    display.print("?");
  }
}

// -----------------------------------------
void getWeather() {
  HTTPClient http;

  http.begin(weatherURL);

  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();

    const size_t capacity = 1024;
    StaticJsonDocument<capacity> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float temp = doc["current_weather"]["temperature"];
      float wind = doc["current_weather"]["windspeed"];
      int code  = doc["current_weather"]["weathercode"];

      display.clearDisplay();

      // Überschrift
      display.setTextSize(1);
      display.setCursor(40, 30);
      display.print("Muenchen");

      // Temperatur
      display.setCursor(0, 40);
      display.setTextSize(1);
      display.print("Temp: ");
      display.print(temp, 1);
      display.print(" C");

      // Wind
      display.setTextSize(1);
      display.setCursor(0, 50);
      display.print("Wind: ");
      display.print(wind, 1);
      display.print(" km/h");

      // Wettericon rechts anzeigen
      drawWeatherIcon(code, 90, 20);

      display.display();
    }
  }

  http.end();
}

// -----------------------------------------
void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // WLAN starten
  display.setCursor(0, 0);
  display.print("Verbinde WLAN...");
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("WLAN verbunden!");
  display.display();

  delay(800);

  getWeather();
}

// -----------------------------------------
void loop() {
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 300000) {  // alle 5 Minuten
    getWeather();
    lastUpdate = millis();
  }
}