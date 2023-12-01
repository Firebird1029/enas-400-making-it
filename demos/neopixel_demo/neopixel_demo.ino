#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(36, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(50);
  strip.show();  // Initialize all pixels to 'off'
}

void loop() {
  // theaterChase(strip.Color(127, 127, 127), 30);  // White
  // theaterChase(strip.Color(127, 0, 0), 30); // Red
  chase(strip.Color(0, 0, 127), 30);  // Blue
}

void chase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) {  // do 10 cycles of chasing
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);

      strip.show();

      delay(random(wait, wait - 10));

      if (random(0, 100) <= 1) {
        delay(5000);
      }

      strip.setPixelColor(i, 0);
    }

    for (uint16_t i = strip.numPixels(); i > 0; i--) {
      strip.setPixelColor(i, c);

      strip.show();

      delay(random(wait, wait - 10));

      if (random(0, 100) <= 1) {
        delay(5000);
      }

      strip.setPixelColor(i, 0);
    }
  }
}
