#include "animate.h"
#include "display.h"

// Global variables and constants
AnimatedGIF gif;
GIFContext gifContext = {&oled, nullptr, 0, 0}; // Context for GIF drawing
// Assume maximum canvas size for all GIFs
const size_t maxCanvasWidth = GIF_WIDTH; // Max width of GIFs
const size_t maxCanvasHeight = GIF_HEIGHT; // Max height of GIFs
const size_t frameBufferSize = maxCanvasWidth * maxCanvasHeight * 2;  // 2 bytes per pixel (RGB565)

GIFData gifFiles[] = {
    { (uint8_t*)LOOK_LEFT_RIGHT_EMOTE, sizeof(LOOK_LEFT_RIGHT_EMOTE) },
    { (uint8_t*)LOOK_UP_DOWN_EMOTE, sizeof(LOOK_UP_DOWN_EMOTE) },
    { (uint8_t*)UWU_EMOTE, sizeof(UWU_EMOTE) },
    { (uint8_t*)SLEEPY_EMOTE, sizeof(SLEEPY_EMOTE) },
    { (uint8_t*)SIGH_EMOTE, sizeof(SIGH_EMOTE) },
    { (uint8_t*)SHOCK_EMOTE, sizeof(SHOCK_EMOTE) },
    { (uint8_t*)PERVE_EMOTE, sizeof(PERVE_EMOTE) },
    { (uint8_t*)MISCHIEF_EMOTE, sizeof(MISCHIEF_EMOTE) },
    { (uint8_t*)LAUGH_EMOTE, sizeof(LAUGH_EMOTE) },
    { (uint8_t*)KISSY_EMOTE, sizeof(KISSY_EMOTE) },
    { (uint8_t*)JUDGE_EMOTE, sizeof(JUDGE_EMOTE) },
    { (uint8_t*)DIZZY_EMOTE, sizeof(DIZZY_EMOTE) },
    { (uint8_t*)CRY_EMOTE, sizeof(CRY_EMOTE) },
    { (uint8_t*)ANGRY_EMOTE, sizeof(ANGRY_EMOTE) },
    // Add other GIFs here (up to 15)
};
void printMemoryStats() {
    Serial.printf("Free heap: %u bytes\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    Serial.printf("Free PSRAM: %u bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void initializeGIF() { 
  gif.begin(GIF_PALETTE_RGB565_LE); 
    // Allocate shared frame buffer in PSRAM (only once during initialization)
  if (gifContext.sharedFrameBuffer == nullptr) {
    gifContext.sharedFrameBuffer = (uint8_t *)heap_caps_malloc(frameBufferSize, MALLOC_CAP_8BIT);
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
    gifContext.oled->setAddrWindow(gifContext.offsetX + pDraw->iX, gifContext.offsetY + pDraw->iY, pDraw->iWidth, pDraw->iHeight);
  }
      // Write the current line of pixels to the display
    uint16_t *pixels = (uint16_t *)pDraw->pPixels;  // Cast pixel data to 16-bit format
    gifContext.oled->writePixels(pixels, pDraw->iWidth);      // Write the entire line at once

       // For the last line of the frame, end the write transaction
    if (pDraw->y == pDraw->iHeight - 1) {
        gifContext.oled->endWrite();  // End the display write transaction
    }
}

void playGIF(uint8_t *gifData, size_t gifSize, bool loop = false) {
  // Attempt to open the GIF file from the given data and size, and set up
  // drawing using GIFDraw. If the open operation fails, the function will
  // return early and print an error message.
  if (!gif.open(gifData, gifSize, GIFDraw)) {
    Serial.println("Error: Failed to open GIF file."); // Print an error message
    cleanupGIFContext(); // Ensure cleanup happens on failure
    return; // Exit the function early if the GIF fails to open
  }

  // Calculate the offset (position) where the GIF should be drawn on the screen
  gifContext.offsetX = (oled.width() - gif.getCanvasWidth()) / 2;  // Center horizontally
  gifContext.offsetY = (oled.height() - gif.getCanvasHeight()) / 2; // Center vertically
  // Calculate the size of the framebuffer required to store one frame of the GIF
  size_t currentFrameBufferSize = gif.getCanvasWidth() * (gif.getCanvasHeight() + 2); // Adjust as needed

  // Only reallocate the frame buffer if the size has changed
  if (gifContext.sharedFrameBuffer == nullptr || currentFrameBufferSize != frameBufferSize) {
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

  // Cleanup after the GIF finishes playing
  cleanupGIFContext();
}

// Function to play a GIF// Function to play a random GIF
void playRandomGIF() {
  while (true) { // Infinite loop to continuously play random GIFs
    // Randomly select a GIF file
    int randomIndex = random(0, TOTAL_GIFS); // Get a random index (0 to NUM_GIFS-1)

    uint8_t* gifData = gifFiles[randomIndex].data;
    size_t gifSize = gifFiles[randomIndex].size;

    // Call playGIF to handle the actual playback of the selected GIF
    playGIF(gifData, gifSize, false);

    // Define the delay time (in milliseconds)
    unsigned long delayTime = random(2000, 4000);  // 2-5 seconds delay, adjust as needed
    // Store the current time to manage the delay
    unsigned long startDelayTime = millis();
    
    // Play the specific GIF (e.g., loading GIF or indicator) and track its duration
    playGIF((uint8_t*)REST_EMOTE, sizeof(REST_EMOTE), false); 

    // Wait for the delay period, but ensure the specific GIF plays fully
    while (millis() - startDelayTime < delayTime) {
      // Optionally update your display here
      // For example, you might want to update a progress bar or refresh the screen
      // Example: oled.drawProgressBar(10, 10, 100, 10, (millis() - startDelayTime) / delayTime * 100);
      // Or update an indicator showing how much time is left on the delay
    }

    // After the delay time, continue to play the next random GIF
  }
}