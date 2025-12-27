#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>

#define QMI8658_ADDR 0x6B

TFT_eSPI tft = TFT_eSPI();

// ---------------- I2C helpers ----------------
void qmiWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void qmiRead(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)QMI8658_ADDR, (uint8_t)len);
  for (int i = 0; i < len; i++) buf[i] = Wire.read();
}

// ---------------- QMI8658 init ----------------
bool qmiInit() {
  uint8_t who;
  qmiRead(0x00, &who, 1);   // WHO_AM_I

  if (who != 0x05) return false;

  qmiWrite(0x02, 0x60); // accel ODR 250Hz ±2g
  qmiWrite(0x03, 0x60); // gyro ODR 250Hz ±256dps
  qmiWrite(0x08, 0x03); // enable accel + gyro

  return true;
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(6, 7);   // SDA, SCL for Waveshare S3 board

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setFreeFont(&FreeSans12pt7b);

  tft.drawString("QMI8658 TEST", 120, 30);

  if (!qmiInit()) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Sensor FAIL", 120, 120);
    while (1);
  }

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Sensor OK", 120, 120);
  delay(1000);
}

// ---------------- Loop ----------------
void loop() {
  uint8_t buf[6];
  qmiRead(0x35, buf, 6);

  int16_t ax = (buf[1] << 8) | buf[0];
  int16_t ay = (buf[3] << 8) | buf[2];
  int16_t az = (buf[5] << 8) | buf[4];

  float axg = ax / 16384.0;
  float ayg = ay / 16384.0;
  float azg = az / 16384.0;

  tft.fillRect(0, 70, 240, 170, TFT_BLACK);

  tft.setCursor(40, 90);
  tft.printf("AX: %.2f g\n", axg);

  tft.setCursor(40, 130);
  tft.printf("AY: %.2f g\n", ayg);

  tft.setCursor(40, 170);
  tft.printf("AZ: %.2f g\n", azg);

  delay(200);
}
