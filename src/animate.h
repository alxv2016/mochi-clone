#ifndef ANIMATE_H
#define ANIMATE_H

#include <AnimatedGIF.h>
#include "display.h"
#include "emojis.h"

extern AnimatedGIF gif;

struct GIFContext {
  Adafruit_SSD1351 *oled;
  uint8_t *sharedFrameBuffer;
  int offsetX;
  int offsetY;
};

struct GIFData {
  uint8_t* data;
  size_t size;
};

extern GIFData randomEmotes[];
extern GIFData restingEmotes[];

enum State {
  RESTING,
  RANDOM,
  DIZZY,
  TILTING
};

// GIF SETTINGS
#define GIF_HEIGHT 128
#define GIF_WIDTH 128
// Frame delay timing (15 FPS)
#define FRAME_DELAY_MICROSECONDS (1000000 / 15)
// Define the macro to count the number of GIFs in each array
#define RANDOM_EMOTES (sizeof(randomEmotes) / sizeof(randomEmotes[0]))
#define RESTING_EMOTES (sizeof(restingEmotes) / sizeof(restingEmotes[0]))
// Array of GIFs
#define REST_EMOTE rest
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

void initializeGIFContext();
void playGIF(uint8_t* gifData, size_t gifSize, bool loop);
void printMemoryStats(void);
void initializeEmotes(void);

#endif