#ifndef SCREEN_ANIMATE_H
#define SCREEN_ANIMATE_H

#include <bb_spi_lcd.h>
#include <AnimatedGIF.h>
#include "emojis.h"

// Global objects
extern BB_SPI_LCD oled;
extern AnimatedGIF gif;

struct GIFContext {
  BB_SPI_LCD *oled;
  uint8_t *frameBuffer;
  int offsetX;
  int offsetY;
};

struct GIFData {
  uint8_t* data; // Pointer to the GIF data
  size_t size;   // Size of the GIF data
};

// SSD1351 PINOUT for ESP32-S3
// GND | GND (0V) // Common
// VCC | 5V or 3.3V // Better to power with 5V if display PCB supports it
// DIN | MOSI // SPI data
// SCK | SCLK // SPI clock
// DC | DC // Distinguish between a command or its data
// RST | RST // Hardware reset, can connect to MCU RST pin as well
// CS | CS // Chip select, Set to -1 if for manually use with multiple displays

// Pin configuration
#define MISO_PIN -1
#define MOSI_PIN 9
#define CLK_PIN 3
#define CS_PIN 8
#define DC_PIN 18
#define RST_PIN 17

// SPI settings
#define SPI_FREQUENCY 20000000

// Display dimensions
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 128

// GIF SETTINGS
#define GIF_FPS 15
#define TOTAL_GIFS 6
// Array of GIFs
#define REST_EMOTE rest
#define LOOK_EMOTE look
#define LAUGH_EMOTE laugh
#define CRY_EMOTE cry
#define SHOCK_EMOTE shock
#define KIDDY_EMOTE kiddy

// Function declarations
void cleanupGIFContext(void);
void GIFDraw(GIFDRAW *pDraw);
void playGIF(uint8_t* gifData, size_t gifSize, bool loop);
void playRandomGIF(void);

#endif // SCREEN_ANIMATE_H