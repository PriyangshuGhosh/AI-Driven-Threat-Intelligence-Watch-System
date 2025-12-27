#include <TFT_eSPI.h>
#include <WiFi.h>
#include "time.h"

// ================= CONFIG =================
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

const char* ssid     = "Test";
const char* password = "12335678";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;   // IST
const int   daylightOffset_sec = 0;
// =========================================

TFT_eSPI tft = TFT_eSPI();

void drawWatchFace() {
  tft.fillScreen(TFT_BLACK);

  // Outer circle
  tft.drawCircle(120, 120, 118, TFT_DARKGREY);
  tft.drawCircle(120, 120, 116, TFT_DARKGREY);

  // Hour marks
  for (int i = 0; i < 12; i++) {
    float angle = i * 30 * DEG_TO_RAD;
    int x1 = 120 + cos(angle) * 95;
    int y1 = 120 + sin(angle) * 95;
    int x2 = 120 + cos(angle) * 105;
    int y2 = 120 + sin(angle) * 105;
    tft.drawLine(x1, y1, x2, y2, TFT_WHITE);
  }
}

void drawTime(struct tm *timeinfo) {
  char timeBuf[9];
  sprintf(timeBuf, "%02d:%02d:%02d",
          timeinfo->tm_hour,
          timeinfo->tm_min,
          timeinfo->tm_sec);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(3);
  tft.drawString(timeBuf, 120, 110);

  char dateBuf[16];
  sprintf(dateBuf, "%02d-%02d-%04d",
          timeinfo->tm_mday,
          timeinfo->tm_mon + 1,
          timeinfo->tm_year + 1900);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(dateBuf, 120, 150);
}

void connectWiFi() {
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("Connecting WiFi", 120, 120);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
  }

  tft.fillScreen(TFT_BLACK);
  tft.drawString("WiFi Connected", 120, 120);
  delay(800);
}

void setup() {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  connectWiFi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  drawWatchFace();
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Time sync failed");
    delay(1000);
    return;
  }

  drawWatchFace();
  drawTime(&timeinfo);

  delay(1000);
}
