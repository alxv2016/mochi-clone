#ifndef ANIMATE_H
#define ANIMATE_H

#include <AnimatedGIF.h>
#include "display.h"
#include "emojis.h"

// Global objects
extern AnimatedGIF gif;

struct GIFContext {
  Adafruit_SSD1351 *oled;
  uint8_t *sharedFrameBuffer;
  int offsetX;
  int offsetY;
};

struct GIFData {
  uint8_t* data; // Pointer to the GIF data
  size_t size;   // Size of the GIF data
};
// Declare the array (no definition here, just a declaration)
extern GIFData gifFiles[];

// GIF SETTINGS
#define GIF_FPS 15
#define GIF_HEIGHT 128
#define GIF_WIDTH 128
// Define the macro to count the number of GIFs in the array
#define TOTAL_GIFS (sizeof(gifFiles) / sizeof(gifFiles[0]))
#define REST_EMOTE rest
// Array of GIFs
#define LOOK_LEFT_RIGHT_EMOTE look_left_right
#define LOOK_UP_DOWN_EMOTE look_up_down
#define UWU_EMOTE uwu
#define SLEEPY_EMOTE sleepy
#define SIGH_EMOTE sigh
#define SHOCK_EMOTE shock
#define PERVE_EMOTE perve
#define MISCHIEF_EMOTE mischief
#define LAUGH_EMOTE laugh
#define KISSY_EMOTE kissy
#define JUDGE_EMOTE judge
#define DIZZY_EMOTE dizzy
#define CRY_EMOTE cry
#define ANGRY_EMOTE angry

// Function declarations
void cleanupGIFContext(void);
void GIFDraw(GIFDRAW *pDraw);
void playGIF(uint8_t* gifData, size_t gifSize, bool loop);
void playRandomGIF(void);
void initializeGIF(void) ;
void printMemoryStats(void);

#endif