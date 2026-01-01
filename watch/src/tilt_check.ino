#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>

#define QMI8658_ADDR 0x6B
#define R 120             // Radius
#define CENTER 120        // Center X and Y
#define CONTENT_LINES 100 
#define LINE_H 28

TFT_eSPI tft = TFT_eSPI();
float scrollY = 0, velocity = 0;
const float sensitivity = 2.2, friction = 0.92, deadzone = 0.05;

void qmiWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(reg); Wire.write(val);
  Wire.endTransmission();
}

void setup() {
  Wire.begin(6, 7);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  qmiWrite(0x02, 0x60); // Enable Accelerometer
  qmiWrite(0x08, 0x03); // 50Hz ODR
}

void drawContent() {
  tft.startWrite(); // Lock SPI for faster batch drawing
  
  for (int i = 0; i < CONTENT_LINES; i++) {
    int itemY = (i * LINE_H) - (int)scrollY;

    // Only draw if line is within the circular screen bounds
    if (itemY > -LINE_H && itemY < 240) {
      // Calculate available width at this Y (Pythagoras)
      int dy = abs(CENTER - (itemY + LINE_H / 2));
      int halfWidth = (dy < R) ? sqrt(R * R - dy * dy) : 0;
      
      // Calculate starting X to center the text in the circle's chord
      int startX = CENTER - halfWidth + 10; 
      int maxWidth = (halfWidth * 2) - 20;

      // Clear the line background specifically to prevent flickering
      tft.fillRect(CENTER - halfWidth, itemY, halfWidth * 2, LINE_H, TFT_BLACK);

      if (maxWidth > 40) { // Only draw if there's enough room
        tft.setTextColor(tft.color565(0, 255, 65));
        tft.setCursor(startX, itemY + 5);
        tft.setTextSize(maxWidth < 100 ? 1 : 2); // Simple dynamic scaling
        tft.printf("LOG_%02d: PRIYANGSHU", i);
      }
    }
  }
  
  // Decorative Border
  tft.drawSmoothCircle(CENTER, CENTER, 118, tft.color565(0, 80, 0), TFT_BLACK);
  tft.endWrite();
}

void loop() {
  uint8_t buf[6];
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(0x35);
  Wire.endTransmission(false);
  Wire.requestFrom(QMI8658_ADDR, 6);
  for (int i = 0; i < 6; i++) buf[i] = Wire.read();

  int16_t ay_raw = (buf[3] << 8) | buf[2];
  float ay = ay_raw / 16384.0;

  if (abs(ay) > deadzone) velocity += (ay * sensitivity);
  velocity *= friction;
  scrollY += velocity;

  // Constraints
  if (scrollY < 0) { scrollY = 0; velocity = 0; }
  float maxScroll = (CONTENT_LINES * LINE_H) - 200;
  if (scrollY > maxScroll) { scrollY = maxScroll; velocity = 0; }

  drawContent();
  delay(5); 
}