#ifndef DISPLAY_H
#define DISPLAY_H
// Using Adafruit SSD1351 library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

extern Adafruit_SSD1351 oled;
// Defined colors optimal for OLED screen
#define TFT_BLACK 0x0000
#define TFT_WHITE 0xffff
#define TFT_GREY 0x5AEB

// Display is using a Waveshare 1.5" RGB OLED display
// https://www.waveshare.com/wiki/1.5inch_RGB_OLED_Module
// DISPLAY PINs CORRESPONDENCE to ESP32
// GND -> GND (0V) // Common
// VCC -> 5V or 3.3V // Better to power with 5V if display PCB supports it
// DIN -> MOSI // SPI data
// SCK -> SCLK // SPI clock
// DC -> DC // Distinguish between a command or its data
// RST -> RST // Hardware reset, can connect to MCU RST pin as well
// CS -> CS // Chip select, Set to -1 if for manually use with multiple displays

// Pin configuration for SEEED XIAO ESP32-S3
#define MOSI_PIN 9
#define SCLK_PIN 7
#define CS_PIN 2
#define DC_PIN 4
#define RST_PIN 1

// SPI fequency according to display
#define SPI_FREQUENCY 20000000
// Display dimensions
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 128

void initializeOLED();
void displayBootMessage(const char *message);

#endif