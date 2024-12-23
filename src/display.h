#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

// // Global objects
extern Adafruit_SSD1351 oled;

#define TFT_BLACK 0x0000
#define TFT_GREEN 0x07e0
#define TFT_RED 0xf800
#define TFT_BLUE 0x001f
#define TFT_CYAN 0x07ff
#define TFT_YELLOW 0xffe0
#define TFT_MAGENTA 0xf81f
#define TFT_WHITE 0xffff
#define TFT_GREY 0x5AEB
#define TFT_ORANGE 0xbbc0

// SSD1351 PINOUT for ESP32-S3
// GND | GND (0V) // Common
// VCC | 5V or 3.3V // Better to power with 5V if display PCB supports it
// DIN | MOSI // SPI data
// SCK | SCLK // SPI clock
// DC | DC // Distinguish between a command or its data
// RST | RST // Hardware reset, can connect to MCU RST pin as well
// CS | CS // Chip select, Set to -1 if for manually use with multiple displays

// Pin configuration
#define MOSI_PIN 9
#define SCLK_PIN 7
#define CS_PIN 2
#define DC_PIN 4
#define RST_PIN 1

// SPI fequency
#define SPI_FREQUENCY 20000000
// Display dimensions
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 128
#define DISPLAY_BRIGHTNESS 0

void initializeOLED();
void displayBootMessage(const char *message);

#endif