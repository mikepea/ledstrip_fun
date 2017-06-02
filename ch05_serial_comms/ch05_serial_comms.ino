#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define NUM_LEDS 18

#define BAUD_RATE 115200

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

uint32_t last_update = 0;
uint32_t ms_per_pixel_move = 100;
uint8_t pixel = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  strip.begin();
  strip.show();
}

uint32_t convertByteToColor(uint8_t a) {
  uint8_t r,g,b;
  r = ( a & 0b11110000 );
  g = ( a & 0b00111100 ) << 2;
  b = ( a & 0b00001111 ) << 4;
  return strip.Color(r,g,b);
}

bool timeToRefreshStrip() {
  uint32_t now = millis();
  if ( now - last_update < ms_per_pixel_move ) return false;
  last_update = now;
  return true;
}

uint8_t incoming_byte = 0;
uint32_t color = 0;

void loop() {
  if ( timeToRefreshStrip() ) {
    if (Serial.available() > 0) {
      incoming_byte = Serial.read();
      color = convertByteToColor(incoming_byte);
      Serial.print("color: "); Serial.println(incoming_byte);
    } else {
      color = 0;
    }
    uint8_t last_pixel = ( pixel + strip.numPixels() - 1 ) % strip.numPixels();
    strip.setPixelColor(last_pixel, strip.Color(0, 0, 0));
    strip.setPixelColor(pixel, color);
    strip.show();
    pixel = (pixel + 1) % strip.numPixels();
  }
}
