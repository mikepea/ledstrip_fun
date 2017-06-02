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

uint16_t colorPos = 0;
uint16_t byteCounter = 0;
bool readIntoBuffer = false;


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

bool getDataFromSerial() {
  uint8_t bytesRead = 0;
  while ( Serial.available() > 0 ) {
    uint8_t c = Serial.read();
    processIncomingByte(c);
    bytesRead++;
    if ( bytesRead > 50 ) {
      break; // overflow :/
    }
  }
}


uint8_t convertAsciiHexToBin(uint8_t hexDigit) {
  if ( hexDigit >= 97 && hexDigit <= 102 ) {
    // a-f
    return (hexDigit - 87);
  } else if ( hexDigit >= 65 && hexDigit <= 70 ) {
    // A-F
    return (hexDigit - 55);
  } else if ( hexDigit >= 48 && hexDigit <= 57 ) {
    // 0-9
    return (hexDigit - 48);
  } else {
    // fail
    return 255;
  }
}

void processIncomingByte(uint8_t byte) {
  if ( byte == 'Y' ) {
    readIntoBuffer = true;
    colorPos = 0;
    byteCounter = 0;
    return;
  }
  if ( byte == 'Z' ) {
    // end of transmission
    readIntoBuffer = false;
    // fill rest with zeros
    for ( uint16_t i=colorPos; i < strip.numPixels(); i++ ) {
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
    return;
  }
  if ( colorPos >= strip.numPixels() ) {
    Serial.println("==== too much serial data sent!");
    readIntoBuffer = false;
    return;
  }
  if ( readIntoBuffer && ( colorPos < strip.numPixels() ) ) {
    uint8_t val = convertAsciiHexToBin(byte);
    if ( val != 255 ) {
      processRGBDataIntoPixel(val);
    }
  }
}

// build up a pixel's RGB values by processing each serial 'hex' value
// into hi/lo bits of an 8bit color, then set the pixel when we're ready.
uint8_t r, g, b; // global, since we morph them with processIncomingByte.
void processRGBDataIntoPixel(uint8_t val) {
	switch ( byteCounter ) {
		case 0: // high octet red
			r = val << 4;
			byteCounter++;
			break;
		case 1: // low octet red
			r += val;
			byteCounter++;
			break;
		case 2: // high octet green
			g = val << 4;
			byteCounter++;
			break;
		case 3: // low octet green
			g += val;
			byteCounter++;
			break;
		case 4: // high octet blue
			b = val << 4;
			byteCounter++;
			break;
		case 5: // low octet blue
			b += val;
			// end of color, so set it and clear buffer.
			strip.setPixelColor(colorPos, r, g, b);
			r = 0; g = 0; b = 0; // set to black/off for rewriting
			byteCounter = 0;
			colorPos++;
			break;
	}
}

uint8_t incoming_byte = 0;
uint32_t color = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  strip.begin();
  strip.show();
}

void loop() {
  getDataFromSerial();
  if ( timeToRefreshStrip() ) {
    strip.show();
  }
}
