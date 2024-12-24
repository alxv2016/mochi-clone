#include "display.h"

Adafruit_SSD1351 oled = Adafruit_SSD1351(DISPLAY_WIDTH, DISPLAY_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);
// Initialize the OLED display
void initializeOLED() {
  SPI.begin(SCLK_PIN, -1, MOSI_PIN, CS_PIN);
  oled.begin();
  // Additional check to ensure the display is working
  if (oled.width() != DISPLAY_WIDTH || oled.height() != DISPLAY_HEIGHT) {
    Serial.println("Error: Display failed to initialize.");
    return;
  }

  oled.fillScreen(TFT_BLACK);
  oled.setTextColor(TFT_WHITE);
  Serial.println("Display initialized");
}

// Display a message on the screen
void displayBootMessage(const char *message) {
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  oled.setTextSize(1); 
  oled.getTextBounds(message, 0, 0, &x1, &y1, &textWidth, &textHeight);

  int16_t centeredX = (DISPLAY_WIDTH - textWidth) / 2;
  int16_t centeredY = (DISPLAY_HEIGHT - textHeight) / 2;

  oled.setCursor(centeredX, centeredY);
  oled.println(message);
}