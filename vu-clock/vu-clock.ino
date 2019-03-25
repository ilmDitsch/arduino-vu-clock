#include <Time.h>
#include <TimeLib.h>

#include <DS1307RTC.h>

#include <Wire.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PWM_OUT_PIN_HRS 3
#define PWM_OUT_PIN_MIN 5
#define PWM_OUT_PIN_SEC 6

#define MAX_VALUE_HRS 235
#define MAX_VALUE_MIN 240
#define MAX_VALUE_SEC 237

#define LED_OUT_PIN 2
#define LED_COUNT 3

#define LED_IDX_HRS 0
#define LED_IDX_MIN 1
#define LED_IDX_SEC 2

#define BTN_IN_PLUS 7
#define BTN_IN_MINUS 8

tmElements_t tm;

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, LED_OUT_PIN, NEO_RGB + NEO_KHZ800);

void setup() {
  pinMode(BTN_IN_PLUS, INPUT_PULLUP);
  pinMode(BTN_IN_MINUS, INPUT_PULLUP);

  //Use these lines to calibrate MAX_VALUES
  //analogWrite(PWM_OUT_PIN_HRS, map(12, 0, 12, 0, MAX_VALUE_HRS));
  //analogWrite(PWM_OUT_PIN_MIN, map(60, 0, 60, 0, MAX_VALUE_MIN));
  //analogWrite(PWM_OUT_PIN_SEC, map(60, 0, 60, 0, MAX_VALUE_SEC));

  leds.begin();
  leds.setBrightness(100);
  leds.show(); // Initialize all leds to 'off'
  
  bootAnimation();
}

void loop() {
  if (!updateMeterTime()) {
    analogWrite(PWM_OUT_PIN_HRS, map(6, 0, 12, 0, MAX_VALUE_HRS));
    analogWrite(PWM_OUT_PIN_MIN, map(30, 0, 60, 0, MAX_VALUE_MIN));
    analogWrite(PWM_OUT_PIN_SEC, map(30, 0, 60, 0, MAX_VALUE_SEC));
    
    //could not read time from rtc
    if (RTC.chipPresent() && setTimeFromCompile() && RTC.write(tm)) {
      //new time written
    } else {
      //to add error handler here
      //set LEDs to red?
    }

    delay(10000);
    
  }
  
  delay(100);
}

bool updateMeterTime() {

  if (RTC.read(tm)) {

    int8_t h = (tm.Hour == 12) ? 12 : tm.Hour % 12;

    analogWrite(PWM_OUT_PIN_HRS, map(h, 0, 12, 0, MAX_VALUE_HRS));
    analogWrite(PWM_OUT_PIN_MIN, map(tm.Minute, 0, 60, 0, MAX_VALUE_MIN));
    analogWrite(PWM_OUT_PIN_SEC, map(tm.Second, 0, 60, 0, MAX_VALUE_SEC));

    return true;
  } else {
    return false;
  }
}

void bootAnimation() {
  
  animateLeds0ToColor();
  
  animateMeter0ToMax(PWM_OUT_PIN_HRS, MAX_VALUE_HRS);
  animateMeter0ToMax(PWM_OUT_PIN_MIN, MAX_VALUE_MIN);
  animateMeter0ToMax(PWM_OUT_PIN_SEC, MAX_VALUE_SEC);
  delay(2000);
}

//--- Meter Animations ---
void animateMeter0ToMax(uint8_t pin, uint8_t maxValue) {
  for (uint8_t value = 0; value <= maxValue; value++) {
    analogWrite(pin, value);
    delay(3);
  }
}

//--- LED Animation ---
void animateLeds0ToColor() {
  
  for (uint8_t value = 0; value <= 254; value++) {

    
    leds.setPixelColor(LED_IDX_HRS, leds.Color(map(value,0,255,0,200), map(value,0,255,0,70), map(value,0,255,0,0)));
    leds.setPixelColor(LED_IDX_MIN, leds.Color(map(value,0,255,0,200), map(value,0,255,0,70), map(value,0,255,0,0)));
    leds.setPixelColor(LED_IDX_SEC, leds.Color(map(value,0,255,0,200), map(value,0,255,0,70), map(value,0,255,0,0)));
    leds.show();
    delay(5);
  }
}

//--- Time Setup ---
bool setTimeFromCompile() {
  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    // and configure the RTC with this info
    if (RTC.write(tm)) {
      return true;
    }
  }
  return false;
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec;
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  return true;
}
