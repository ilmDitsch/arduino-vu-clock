#include <Time.h>
#include <TimeLib.h>

#include <DS1307RTC.h>

#include <Wire.h>

#define PWM_OUT_PIN_HRS 3
#define PWM_OUT_PIN_MIN 5
#define PWM_OUT_PIN_SEC 6

#define MAX_VALUE_HRS 235
#define MAX_VALUE_MIN 240
#define MAX_VALUE_SEC 237

#define LED_OUT_PIN 2
#define LED_COUNT 3

#define BTN_IN_PLUS 7
#define BTN_IN_MINUS 8



void setup() {
  pinMode(BTN_IN_PLUS, INPUT_PULLUP);
  pinMode(BTN_IN_MINUS, INPUT_PULLUP);

  //Use these lines to calibrate MAX_VALUES
  //analogWrite(PWM_OUT_PIN_HRS, map(12, 0, 12, 0, MAX_VALUE_HRS));
  //analogWrite(PWM_OUT_PIN_MIN, map(60, 0, 60, 0, MAX_VALUE_MIN));
  //analogWrite(PWM_OUT_PIN_SEC, map(60, 0, 60, 0, MAX_VALUE_SEC));

  bootAnimation();
}

void loop() {
  if (!updateMeterTime()) {
    //to add error handler here
  }
  
  delay(100);
}

bool updateMeterTime() {
  tmElements_t tm;

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
  animateMeter0ToMax(PWM_OUT_PIN_HRS, MAX_VALUE_HRS);
  animateMeter0ToMax(PWM_OUT_PIN_MIN, MAX_VALUE_MIN);
  animateMeter0ToMax(PWM_OUT_PIN_SEC, MAX_VALUE_SEC);
  delay(2000);
}

void animateMeter0ToMax(uint8_t pin, uint8_t maxValue) {
  for (uint8_t value = 0; value <= maxValue; value++) {
    analogWrite(pin, value);
    delay(3);
  }
}
