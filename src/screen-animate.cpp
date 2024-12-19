#include "screen-animate.h"

BB_SPI_LCD oled;
AnimatedGIF gif;
GIFContext gifContext = {&oled, nullptr, 0, 0};

// Array to store GIFs
GIFData gifFiles[TOTAL_GIFS] = {
    { (uint8_t*)LOOK_EMOTE, sizeof(LOOK_EMOTE) },
    { (uint8_t*)LAUGH_EMOTE, sizeof(LAUGH_EMOTE) },
    { (uint8_t*)CRY_EMOTE, sizeof(CRY_EMOTE) },
    { (uint8_t*)SHOCK_EMOTE, sizeof(SHOCK_EMOTE) },
    { (uint8_t*)KIDDY_EMOTE, sizeof(KIDDY_EMOTE) }
};

// Get the width and height of the GIF canvas (the dimensions of the GIFimage).
int canvasWidth = gif.getCanvasWidth();
int canvasHeight = gif.getCanvasHeight();

void cleanupGIFContext() {
  if (gifContext.frameBuffer) {
    heap_caps_free(gifContext.frameBuffer);
    gifContext.frameBuffer = nullptr;
  }
  gif.close();
}

void GIFDraw(GIFDRAW *pDraw) {
  if (pDraw->y == 0) {
    gifContext.oled->setAddrWindow(gifContext.offsetX + pDraw->iX, gifContext.offsetY + pDraw->iY, pDraw->iWidth, pDraw->iHeight);
  }
  gifContext.oled->pushPixels((uint16_t *)pDraw->pPixels, pDraw->iWidth, DRAW_TO_LCD | DRAW_WITH_DMA);
}

void playGIF(uint8_t *gifData, size_t gifSize, bool loop = false) {
  // Attempt to open the GIF file from the given data and size, and set up
  // drawing using GIFDraw. If the open operation fails, the function will
  // return early and print an error message.
  if (!gif.open(gifData, gifSize, GIFDraw)) {
    Serial.println("Error: Failed to open GIF file."); // Print an error message
    return; // Exit the function early if the GIF fails to open
  }

  // Calculate the offset (position) where the GIF should be drawn on the screen
  gifContext.offsetX = (oled.width() - canvasWidth) / 2;  // Center horizontally
  gifContext.offsetY = (oled.height() - canvasHeight) / 2; // Center vertically
  // Calculate the size of the framebuffer required to store one frame of the GIF
  size_t frameBufferSize = canvasWidth * (canvasHeight + 2); // Adjust as needed

  // Allocate memory for the frame buffer
  gifContext.frameBuffer = (uint8_t *)heap_caps_malloc(frameBufferSize, MALLOC_CAP_8BIT);

  if (!gifContext.frameBuffer) {
    Serial.printf("Memory Error: Failed to allocate %zu bytes\n", frameBufferSize);
    gif.close(); // Close the GIF file before returning
    return; // Exit the function if memory allocation fails
  }

  // Set the drawing type to "cooked" to allow the GIF library to pre-process frames
  gif.setDrawType(GIF_DRAW_COOKED);
  gif.setFrameBuf(gifContext.frameBuffer);

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