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

#define MAX_VALUE_HRS 232
#define MAX_VALUE_MIN 237
#define MAX_VALUE_SEC 238

#define LED_OUT_PIN 2
#define LED_COUNT 3

#define LED_IDX_HRS 0
#define LED_IDX_MIN 1
#define LED_IDX_SEC 2

#define BTN_IN_MINUS 7
#define BTN_IN_PLUS 8

#define SECONDS_A_DAY 86400

#define DAY_R 239
#define DAY_G 174
#define DAY_B 136

#define NIGHT_R 39
#define NIGHT_G 29
#define NIGHT_B 147

#define SPREAD 0.05

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
  /*
  analogWrite(PWM_OUT_PIN_HRS, map(12, 0, 12, 0, MAX_VALUE_HRS));
  analogWrite(PWM_OUT_PIN_MIN, map(60, 0, 60, 0, MAX_VALUE_MIN));
  analogWrite(PWM_OUT_PIN_SEC, map(60, 0, 60, 0, MAX_VALUE_SEC));
  delay(100000);
  */

  leds.begin();
  leds.setBrightness(75);
  leds.show(); // Initialize all leds to 'off'
  
  bootAnimation();
}

void loop() {
  //Buttons pressed?
  checkButtons();
  
  //Update Time Meters
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

  //Update LEDs
  uint32_t seconds = secondsSinceMidnight();

  setLedColor(seconds, LED_IDX_HRS, SPREAD * -1.0);
  setLedColor(seconds, LED_IDX_MIN, 0.0);
  setLedColor(seconds, LED_IDX_SEC, SPREAD);

  leds.show();

  delay(100);
}

bool updateMeterTime() {

  if (RTC.read(tm)) {

    writeoutTime(true);

    return true;
  } else {
    return false;
  }
}

bool shouldAnimateHours = false;
bool shouldAnimateMinutes = false;
bool shouldAnimateSeconds = false;

void writeoutTime(bool animated) {
  int8_t h = (tm.Hour == 12) ? 12 : tm.Hour % 12;
  
  if (animated && h == 0) {
    if (shouldAnimateHours) {
      shouldAnimateHours = false;
      animateMeterMaxTo0(PWM_OUT_PIN_HRS, MAX_VALUE_HRS);
    }
  } else {
    analogWrite(PWM_OUT_PIN_HRS, map(h, 0, 12, 0, MAX_VALUE_HRS));
    shouldAnimateHours = true;
  }

  if (animated && tm.Minute == 0) {
    if (shouldAnimateMinutes) {
      shouldAnimateMinutes = false;
      animateMeterMaxTo0(PWM_OUT_PIN_MIN, MAX_VALUE_MIN);
    }
  } else {
    analogWrite(PWM_OUT_PIN_MIN, map(tm.Minute, 0, 60, 0, MAX_VALUE_MIN));
    shouldAnimateMinutes = true;
  }
  
  if (animated && tm.Second == 0) {
    if (shouldAnimateSeconds) {
      shouldAnimateSeconds = false;
      animateMeterMaxTo0(PWM_OUT_PIN_SEC, MAX_VALUE_SEC);
    }
  } else {
    analogWrite(PWM_OUT_PIN_SEC, map(tm.Second, 0, 60, 0, MAX_VALUE_SEC));
    shouldAnimateSeconds = true;
  }
}

void bootAnimation() {
  delay(1000);
  
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

void animateMeterMaxTo0(uint8_t pin, uint8_t maxValue) {
  for (uint8_t value = maxValue; value > 0; value--) {
    analogWrite(pin, value);
    delay(2);
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

//--- LED DayNight Gradient ---

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

uint32_t secondsSinceMidnight() {
  return ((uint32_t)tm.Hour*3600) + ((uint32_t)tm.Minute*60) + (uint32_t)tm.Second; 
}

//--- Buttons ---

void checkButtons() {
  int btnPlus = digitalRead(BTN_IN_PLUS);
  int btnMinus = digitalRead(BTN_IN_MINUS);

  bool waiting = true;
 
  while (btnPlus == LOW || btnMinus == LOW) { //LOW == Button pressed

    if (waiting) {
      uint32_t waitColor = leds.Color(255, 255, 0);
    
      leds.setPixelColor(LED_IDX_HRS, waitColor);
      leds.setPixelColor(LED_IDX_MIN, waitColor);
      leds.setPixelColor(LED_IDX_SEC, waitColor);

      waiting = false;
    } else {
      //changing time
      if (btnPlus == LOW) {
        uint32_t plusColor = leds.Color(0, 255, 0);
    
        leds.setPixelColor(LED_IDX_HRS, plusColor);
        leds.setPixelColor(LED_IDX_MIN, plusColor);
        leds.setPixelColor(LED_IDX_SEC, plusColor);

        if (tm.Minute < 59) {
          tm.Minute++;
        } else if (tm.Hour < 23) {
          tm.Hour++;
          tm.Minute = 0;
        }

        leds.setPixelColor((tm.Minute%3), leds.Color(70, 255, 127));
       
      } else if (btnMinus == LOW) {
        uint32_t minusColor = leds.Color(255, 0, 0);
    
        leds.setPixelColor(LED_IDX_HRS, minusColor);
        leds.setPixelColor(LED_IDX_MIN, minusColor);
        leds.setPixelColor(LED_IDX_SEC, minusColor);

        if (tm.Minute > 0) {
          tm.Minute--;
        } else if (tm.Hour > 0) {
          tm.Hour--;
          tm.Minute = 59;
        }

        leds.setPixelColor((tm.Minute%3), leds.Color(255, 127, 70));
      }
      RTC.write(tm);
    }
    
    writeoutTime(false);
    leds.show();
    
    delay(500);
    
    btnPlus = digitalRead(BTN_IN_PLUS);
    btnMinus = digitalRead(BTN_IN_MINUS);
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
