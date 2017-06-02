#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define NUM_LEDS 18

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

uint32_t last_update = 0;
uint32_t ms_per_pixel_move = 100;
uint8_t pixel = 0;

void setup() {
  strip.begin();
  strip.show();
}

bool timeToRefreshStrip() {
  uint32_t now = millis();
  if ( now - last_update < ms_per_pixel_move ) return false;
  last_update = now;
  return true;
}


void loop() {
  if ( timeToRefreshStrip() ) {
    uint8_t last_pixel = ( pixel + strip.numPixels() - 1 ) % strip.numPixels();
    strip.setPixelColor(last_pixel, strip.Color(0, 0, 0));
    strip.setPixelColor(pixel, strip.Color(255, 0, 0));
    strip.show();
    pixel = (pixel + 1) % strip.numPixels();
  }
}
