#include "animate.h"
#include "display.h"
#include "motion.h"

// Global variables and constants
AnimatedGIF gif;
GIFContext gifContext = {&oled, nullptr, 0, 0};
ShakeOrientationData mpuData;
// Define maximum canvas size (128x128) for all GIFs 2 bytes per pixel (RGB565)
constexpr size_t frameBufferSize = GIF_WIDTH * GIF_HEIGHT * 2;
// Current state for the emotes
State currentState = RESTING;
// Array of all the random emotes
GIFData randomEmotes[] = {
    {(uint8_t *)UWU_EMOTE, sizeof(UWU_EMOTE)},
    {(uint8_t *)SIGH_EMOTE, sizeof(SIGH_EMOTE)},
    {(uint8_t *)SHOCK_EMOTE, sizeof(SHOCK_EMOTE)},
    {(uint8_t *)PERVE_EMOTE, sizeof(PERVE_EMOTE)},
    {(uint8_t *)MISCHIEF_EMOTE, sizeof(MISCHIEF_EMOTE)},
    {(uint8_t *)LAUGH_EMOTE, sizeof(LAUGH_EMOTE)},
    {(uint8_t *)KISSY_EMOTE, sizeof(KISSY_EMOTE)},
    {(uint8_t *)JUDGE_EMOTE, sizeof(JUDGE_EMOTE)},
    {(uint8_t *)CRY_EMOTE, sizeof(CRY_EMOTE)},
    {(uint8_t *)ANGRY_EMOTE, sizeof(ANGRY_EMOTE)},
    // Add other GIFs here (up to 15)
};
// Array of all the resting emotes
GIFData restingEmotes[] = {
    {(uint8_t *)LOOK_LEFT_RIGHT_EMOTE, sizeof(LOOK_LEFT_RIGHT_EMOTE)},
    {(uint8_t *)REST_EMOTE, sizeof(REST_EMOTE)},
    {(uint8_t *)SLEEPY_EMOTE, sizeof(SLEEPY_EMOTE)},
};
// Check available memory for ram and psram
void printMemoryStats() {
  Serial.printf("Free heap: %u bytes\n",
                heap_caps_get_free_size(MALLOC_CAP_8BIT));
  Serial.printf("Free PSRAM: %u bytes\n",
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void initializeGIFContext() {
  gif.begin(GIF_PALETTE_RGB565_LE);
  if (gifContext.sharedFrameBuffer == nullptr) {
    gifContext.sharedFrameBuffer =
        (uint8_t *)heap_caps_malloc(frameBufferSize, MALLOC_CAP_SPIRAM);
    if (!gifContext.sharedFrameBuffer) {
      Serial.println("Error: Failed to allocate shared frame buffer in PSRAM.");
    }
  }
  printMemoryStats();
}
// Clear the GIF context (Clean up memory) and close the GIF file
static void cleanupGIFContext() {
  if (gifContext.sharedFrameBuffer) {
    heap_caps_free(gifContext.sharedFrameBuffer);
    gifContext.sharedFrameBuffer = nullptr;
  }
  gif.close();
}

static void GIFDraw(GIFDRAW *pDraw) {
  if (pDraw->y == 0) {
    gifContext.oled->startWrite();
    gifContext.oled->setAddrWindow(gifContext.offsetX + pDraw->iX,
                                   gifContext.offsetY + pDraw->iY,
                                   pDraw->iWidth, pDraw->iHeight);
  }
  uint16_t *pixels = (uint16_t *)pDraw->pPixels;
  gifContext.oled->writePixels(pixels, pDraw->iWidth);
  if (pDraw->y == pDraw->iHeight - 1) {
    gifContext.oled->endWrite();
  }
}

static bool handleEmoteStates() {
  captureMPUData();
  mpuData = detectShakeAndOrientation(
      accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
      gyro.gyro.x, gyro.gyro.y, gyro.gyro.z, 0.01, SHAKE_THRESHOLD,
      TURN_THRESHOLD, TILT_THRESHOLD);
  // Detect MPU motion and change emote state, close current active GIF and play current state GIF
  if (mpuData.isShaking && currentState != DIZZY) {
    Serial.println("Interrupt detected. Switching to DIZZY_EMOTE.");
    cleanupGIFContext();
    currentState = DIZZY;
    return true;
  } else if (mpuData.isTilting && currentState != TILTING) {
    Serial.println("Interrupt detected. Switching to TILTING.");
    cleanupGIFContext();
    currentState = TILTING;
    return true;
  }
  return false;
}

void playGIF(uint8_t *gifData, size_t gifSize, bool loop = false) {
  // Return early and print an error message if the GIF fails to open
  if (!gif.open(gifData, gifSize, GIFDraw)) {
    Serial.println("Error: Failed to open GIF file.");
    cleanupGIFContext();
    return;
  }

  // Calculate the offset (position) where the GIF should be drawn on the screen
  gifContext.offsetX = (oled.width() - gif.getCanvasWidth()) / 2;
  gifContext.offsetY = (oled.height() - gif.getCanvasHeight()) / 2;
  // Calculate the size of the framebuffer required to store one frame of the GIF
  size_t currentFrameBufferSize = gif.getCanvasWidth() * (gif.getCanvasHeight() + 2);

  // Only reallocate the frame buffer if the size has changed
  if (gifContext.sharedFrameBuffer == nullptr ||
      currentFrameBufferSize != frameBufferSize) {
    if (gifContext.sharedFrameBuffer) {
      heap_caps_free(gifContext.sharedFrameBuffer);
    }
    gifContext.sharedFrameBuffer =
        (uint8_t *)heap_caps_malloc(currentFrameBufferSize, MALLOC_CAP_8BIT);
    // Exit the function if memory allocation fails
    if (!gifContext.sharedFrameBuffer) {
      Serial.printf("Memory Error: Failed to allocate %zu bytes\n",
                    currentFrameBufferSize);
      cleanupGIFContext();
      return;
    }
  }

  // Set the drawing type to "cooked" to allow the GIF library to pre-process
  gif.setDrawType(GIF_DRAW_COOKED);
  gif.setFrameBuf(gifContext.sharedFrameBuffer);

  const int frameDelay = FRAME_DELAY_MICROSECONDS;
  unsigned long previousTime = 0;
  unsigned long currentTime = 0;

  // Play the GIF frames
  do {
    while (gif.playFrame(false, nullptr)) {
      // Get the current time in microseconds
      currentTime = micros();
      if (handleEmoteStates()) {
        return;
      }
      // Wait for the next frame based on the desired FPS
      if (currentTime - previousTime >= frameDelay) {
        previousTime = currentTime; // Update the previous time
      } else {
        // Wait for the remaining time to meet the target FPS
        delayMicroseconds(frameDelay - (currentTime - previousTime));
        // Update previousTime to the new time after delay
        previousTime = micros();
      }
    }
    // If looping is enabled, reset the GIF to the first frame
    if (loop) {
      gif.reset();
    }
  } while (loop);
  // Cleanup memory after the GIF finishes playing
  cleanupGIFContext();
}
// Play a random resting GIF
void playRandomRestingGIF() {
  int randomIndex = random(0, RESTING_EMOTES);
  Serial.println("Currently resting, playing random resting GIF.");
  playGIF(restingEmotes[randomIndex].data,
          restingEmotes[randomIndex].size, false);
}
// Play a random GIF
void playRandomGIF() {
  int randomIndex = random(0, RANDOM_EMOTES);
  Serial.println("Currently resting, playing random GIF.");
  playGIF(randomEmotes[randomIndex].data, randomEmotes[randomIndex].size, false);
}

// Flag to track if a GIF is currently playing
void initializeEmotes() {
  // Check if motion is detected and change emote state
  if (mpuData.isShaking) {
    // If shaking is detected, play DIZZY_EMOTE and stop current GIF
    currentState = DIZZY;
    Serial.println(
        "Shaking detected, playing DIZZY_EMOTE.");
    playGIF((uint8_t *)DIZZY_EMOTE, sizeof(DIZZY_EMOTE), false);
    // Switch back to Resting state after playing the DIZZY_EMOTE
    currentState = RESTING;
    return;
  } else if (mpuData.isTilting) {
    // If tilting is detected, play TILTING GIF and stop current GIF
    currentState = TILTING;
    Serial.println("Tilting detected, playing TILTING GIF.");
    playGIF((uint8_t *)LOOK_UP_DOWN_EMOTE, sizeof(LOOK_UP_DOWN_EMOTE), false);
    // Switch back to Resting state after playing the TILTING GIF
    currentState = RESTING;
    return;
  }
  // If not motion is detected play a resting and random emote subsequently
  if (currentState == RESTING) {
    playRandomRestingGIF();
    currentState = RANDOM;
  } else {
    playRandomGIF();
    currentState = RESTING;
  }
}
