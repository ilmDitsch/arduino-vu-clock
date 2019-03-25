#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 2
#define NUM_LEDS 3

#define SECONDS_A_DAY 86400

#define DAY_R 239
#define DAY_G 174
#define DAY_B 136

#define NIGHT_R 39
#define NIGHT_G 29
#define NIGHT_B 147

Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);

void setup() {
  leds.begin();
  leds.setBrightness(100);
  leds.show(); // Initialize all pixels to 'off'
}

void loop() {
  for (uint32_t seconds = 0; seconds <= SECONDS_A_DAY; seconds++) {
    float xRaw = seconds / float(SECONDS_A_DAY);
    float x = gradientMapping(xRaw, 0.0);

    uint32_t color = leds.Color(
      interpolate(x, NIGHT_R, DAY_R),
      interpolate(x, NIGHT_G, DAY_G),
      interpolate(x, NIGHT_B, DAY_B)
    );

    for (uint8_t i=0; i < leds.numPixels(); i++) {
        leds.setPixelColor(i, color);
    }
    leds.show();
    //delay(2);
  }
}

double gradientMapping(float x, float offset) {
  return sq(sin(PI*(x+offset)));
}

uint8_t interpolate(float x, uint8_t startValue, uint8_t endValue) {
  return round(startValue + (x*(endValue-startValue)));
}
