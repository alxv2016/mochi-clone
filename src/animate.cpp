#include "animate.h"
#include "display.h"
#include "movement.h"

// Global variables and constants
AnimatedGIF gif;
GIFContext gifContext = {&oled, nullptr, 0, 0}; // Context for GIF drawing
ShakeOrientationData mpuData;
// Assume maximum canvas size for all GIFs
constexpr size_t maxCanvasWidth = GIF_WIDTH;   // Max width of GIFs
constexpr size_t maxCanvasHeight = GIF_HEIGHT; // Max height of GIFs
constexpr size_t frameBufferSize = maxCanvasWidth * maxCanvasHeight * 2; // 2 bytes per pixel (RGB565)
bool isDizzy = false;
bool isResting = true;

GIFData gifFiles[] = {
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

GIFData restingGifFiles[] = {
    {(uint8_t *)LOOK_LEFT_RIGHT_EMOTE, sizeof(LOOK_LEFT_RIGHT_EMOTE)},
    {(uint8_t *)LOOK_UP_DOWN_EMOTE, sizeof(LOOK_UP_DOWN_EMOTE)},
    {(uint8_t *)REST_EMOTE, sizeof(REST_EMOTE)},
    {(uint8_t *)SLEEPY_EMOTE, sizeof(SLEEPY_EMOTE)},
};


void printMemoryStats() {
  Serial.printf("Free heap: %u bytes\n",
                heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  Serial.printf("Free PSRAM: %u bytes\n",
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void initializeGIF() {
  gif.begin(GIF_PALETTE_RGB565_LE);
  // Allocate shared frame buffer in PSRAM (only once during initialization)
  if (gifContext.sharedFrameBuffer == nullptr) {
    gifContext.sharedFrameBuffer =
        (uint8_t *)heap_caps_malloc(frameBufferSize, MALLOC_CAP_8BIT);
    if (!gifContext.sharedFrameBuffer) {
      Serial.println("Error: Failed to allocate shared frame buffer in PSRAM.");
    }
  }
  printMemoryStats();
}

void cleanupGIFContext() {
  if (gifContext.sharedFrameBuffer) {
    heap_caps_free(gifContext.sharedFrameBuffer);
    gifContext.sharedFrameBuffer = nullptr;
  }
  gif.close();
}

void GIFDraw(GIFDRAW *pDraw) {
  if (pDraw->y == 0) {
    gifContext.oled->startWrite(); // Start the display write transaction
    gifContext.oled->setAddrWindow(gifContext.offsetX + pDraw->iX,
                                   gifContext.offsetY + pDraw->iY,
                                   pDraw->iWidth, pDraw->iHeight);
  }
  // Write the current line of pixels to the display
  uint16_t *pixels =
      (uint16_t *)pDraw->pPixels; // Cast pixel data to 16-bit format
  gifContext.oled->writePixels(pixels,
                               pDraw->iWidth); // Write the entire line at once

  // For the last line of the frame, end the write transaction
  if (pDraw->y == pDraw->iHeight - 1) {
    gifContext.oled->endWrite(); // End the display write transaction
  }
}

void playGIF(uint8_t *gifData, size_t gifSize, bool loop = false) {
  // Attempt to open the GIF file from the given data and size, and set up
  // drawing using GIFDraw. If the open operation fails, the function will
  // return early and print an error message.
  if (!gif.open(gifData, gifSize, GIFDraw)) {
    Serial.println("Error: Failed to open GIF file."); // Print an error message
    cleanupGIFContext(); // Ensure cleanup happens on failure
    return;              // Exit the function early if the GIF fails to open
  }

  // Calculate the offset (position) where the GIF should be drawn on the screen
  gifContext.offsetX =
      (oled.width() - gif.getCanvasWidth()) / 2; // Center horizontally
  gifContext.offsetY =
      (oled.height() - gif.getCanvasHeight()) / 2; // Center vertically
  // Calculate the size of the framebuffer required to store one frame of the
  // GIF
  size_t currentFrameBufferSize =
      gif.getCanvasWidth() * (gif.getCanvasHeight() + 2); // Adjust as needed

  // Only reallocate the frame buffer if the size has changed
  if (gifContext.sharedFrameBuffer == nullptr || currentFrameBufferSize != frameBufferSize) {
    if (gifContext.sharedFrameBuffer) {
      heap_caps_free(gifContext.sharedFrameBuffer);
    }
    gifContext.sharedFrameBuffer = (uint8_t *)heap_caps_malloc(currentFrameBufferSize, MALLOC_CAP_8BIT);
    if (!gifContext.sharedFrameBuffer) {
      Serial.printf("Memory Error: Failed to allocate %zu bytes\n", currentFrameBufferSize);
      cleanupGIFContext();
      return; // Exit the function if memory allocation fails
    }
  }

  // Set the drawing type to "cooked" to allow the GIF library to pre-process frames
  gif.setDrawType(GIF_DRAW_COOKED);
  gif.setFrameBuf(gifContext.sharedFrameBuffer);

  const int targetFPS = GIF_FPS; // Set the target FPS
  const int frameDelay = 1000000 / targetFPS; // Microseconds per frame (1 second = 1000000 microseconds)
  unsigned long previousTime = 0; // Track the time of the previous frame
  unsigned long currentTime = 0;

  // Play the GIF frames
  do {
    while (gif.playFrame(false, nullptr)) {
      currentTime = micros(); // Get the current time in microseconds
      captureMPUData();
      mpuData = detectShakeAndOrientation(accel.acceleration.x, accel.acceleration.y,
                  accel.acceleration.z, gyro.gyro.x, gyro.gyro.y, gyro.gyro.z, 0.01, SHAKE_THRESHOLD, TURN_THRESHOLD);

      if (mpuData.isShaking && !isDizzy) {
        Serial.println("Interrupt detected. Switching to DIZZY_EMOTE.");
        cleanupGIFContext();
        return; // Exit to play the new GIF
      }
      // Wait for the next frame based on the desired FPS
      if (currentTime - previousTime >= frameDelay) {
        previousTime = currentTime; // Update the previous time
      } else {
        // Wait for the remaining time to meet the target FPS
        delayMicroseconds(frameDelay - (currentTime - previousTime));
        previousTime = micros(); // Update previousTime to the new time after delay
      }
    }

    if (loop) {
      gif.reset(); // Reset the GIF to the first frame for looping
    }
  } while (loop);

  isDizzy = false;
  // Cleanup after the GIF finishes playing
  cleanupGIFContext();
}
// Flag to track if a GIF is currently playing
void interactRandomGIF() {
  // Randomly select a GIF file
  if (mpuData.isShaking) {
    isDizzy = true;
    Serial.println("Shaking detected. Playing DIZZY_EMOTE and stopping current GIF.");
    playGIF((uint8_t *)DIZZY_EMOTE, sizeof(DIZZY_EMOTE), false);
    return;
  }

  uint8_t *gifData;
  size_t gifSize;

  if (isResting) {
    int randomRestIndex = random(0, TOTAL_RESTING_GIFS);
    Serial.println("No shaking detected. Playing random resting GIF.");
    gifData = restingGifFiles[randomRestIndex].data;
    gifSize = restingGifFiles[randomRestIndex].size;
    isResting = false;
  } else {
    int randomIndex = random(0, TOTAL_GIFS);
    Serial.println("Shaking stopped. Playing random GIF.");
    gifData = gifFiles[randomIndex].data;
    gifSize = gifFiles[randomIndex].size;
    isResting = true;
  }

  playGIF(gifData, gifSize, false);
}
