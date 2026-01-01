#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>

/** * PRIYANGSHU_OS - Fixed Kinetic Scrolling
 */

#define QMI8658_ADDR 0x6B
#define R 120
#define CENTER 120
#define LINE_H 24      
#define MAX_LINES 20   

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite canvas = TFT_eSprite(&tft);

float scrollY = 0, velocity = 0;
const float friction = 0.92, sensitivity = 3.2; // Optimized for smooth feel
const uint32_t NEON_GREEN = 0x07E4;

void setup() {
  Wire.begin(6, 7);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  canvas.createSprite(240, 240);
  canvas.setTextSize(2);

  // QMI8658 IMU Wakeup & Config
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(0x02); Wire.write(0x60); // Enable Accelerometer
  Wire.endTransmission();
  
  delay(50);
  renderUI(); // Initial draw so screen isn't black
}

void renderUI() {
  canvas.fillSprite(TFT_BLACK); 

  // Draw Static Cyberpunk Border
  canvas.drawSmoothCircle(CENTER, CENTER, 119, NEON_GREEN, TFT_BLACK);
  canvas.drawSmoothCircle(CENTER, CENTER, 115, tft.color565(0, 60, 0), TFT_BLACK);

  for (int i = 0; i < MAX_LINES; i++) {
    // Distribute lines vertically
    int itemY = 60 + (i * LINE_H * 1.8) - (int)scrollY;

    // Calculate distance from center for circular clipping
    int dy = abs(CENTER - (itemY + 8)); 
    
    if (dy < R - 20) {
      // Pythagoras for chord width: w = 2 * sqrt(R^2 - dy^2)
      int availWidth = 2 * sqrt(R * R - dy * dy) - 40;
      
      if (availWidth > 40) {
        canvas.setTextColor(NEON_GREEN);
        canvas.setTextDatum(MC_DATUM); 
        // Logic: if the line is too long for the current circle width, we'd wrap, 
        // but here we center it for the "clean" look you requested.
        canvas.drawString("Happy New Year 2026!", CENTER, itemY);
        canvas.drawString("- Priyangshu\n", CENTER, itemY + 18);
      }
    }
  }
  canvas.pushSprite(0, 0);
}

void loop() {
  // Read IMU (Reading 6 bytes is more stable for QMI series)
  Wire.beginTransmission(QMI8658_ADDR);
  Wire.write(0x35); 
  Wire.endTransmission(false);
  Wire.requestFrom(QMI8658_ADDR, 6);
  
  int16_t ax_raw = (Wire.read() | (Wire.read() << 8));
  int16_t ay_raw = (Wire.read() | (Wire.read() << 8));
  int16_t az_raw = (Wire.read() | (Wire.read() << 8));

  // Use the axis that corresponds to your tilt (usually AY or AX)
  float accel = ay_raw / 16384.0;

  // Kinetic Physics Logic
  if (abs(accel) > 0.05) { 
    velocity += (accel * sensitivity);
  }
  
  velocity *= friction;
  scrollY += velocity;

  // Constraints
  if (scrollY < 0) { scrollY = 0; velocity = 0; }
  if (scrollY > 500) { scrollY = 500; velocity = 0; }

  // High-performance check: Only render if moving or at start
  if (abs(velocity) > 0.02) {
    renderUI();
  }
  
  delay(10); 
}