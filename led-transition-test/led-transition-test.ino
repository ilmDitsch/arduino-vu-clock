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

#define SPREAD 0.05

Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);

void setup() {
  leds.begin();
  leds.setBrightness(100);
  leds.show(); // Initialize all pixels to 'off'
}

void loop() {
  for (uint32_t seconds = 0; seconds <= SECONDS_A_DAY; seconds++) {
    setLedColor(seconds, 0, SPREAD);
    setLedColor(seconds, 1, 0.0);
    setLedColor(seconds, 2, SPREAD * -1.0);
    leds.show();
    //delay(2);
  }
}

void setLedColor(uint32_t seconds, uint8_t ledIndex, float offset) {
  float xRaw = seconds / float(SECONDS_A_DAY);
  float x = gradientMapping(xRaw, offset);

  uint32_t color = leds.Color(
      interpolate(x, NIGHT_R, DAY_R),
      interpolate(x, NIGHT_G, DAY_G),
      interpolate(x, NIGHT_B, DAY_B)
  );

  leds.setPixelColor(ledIndex, color);
}

double gradientMapping(float x, float offset) {
  return sq(sin(PI*(x+offset)));
}

uint8_t interpolate(float x, uint8_t startValue, uint8_t endValue) {
  return round(startValue + (x*(endValue-startValue)));
}
