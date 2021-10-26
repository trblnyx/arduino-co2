//#define DEBUG
/*
  monitorCO2.ino

  A simple CO2 meter using the Adafruit SCD30 breakout 
  Adafruit Feather M0 Basic/Express dev boards
  FeatherWing OLED 128x64 
    
  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Laboratorio de Tecnologías Abiertas, LaTA+
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>
*/

#include <Wire.h>
#include <Adafruit_SCD30.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include "splash.h"

#ifndef DEBUG
#include <Adafruit_SleepyDog.h>
#endif

#define FG_COLOR SH110X_WHITE
#define BG_COLOR SH110X_BLACK

#define VBATPIN  A7

#define SHORT_DELAY 128
#define DELAY 1024
#define LONG_DELAY 4096

#define MEASUREMENT_INTERVAL 2

Adafruit_SCD30 scd30;

Adafruit_SH1107 oled = Adafruit_SH1107(64, 128, &Wire);

boolean led = false;

inline void toggleLED(void) {
  digitalWrite(LED_BUILTIN, led=!led);
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  toggleLED();
  pinMode(VBATPIN, INPUT);

#ifdef DEBUG
  Serial.begin(115200);
  for(char ch=16;!Serial&&ch>0;ch--) {
    toggleLED();
    delay(SHORT_DELAY);
  }
  Serial.println();
  Serial.println(F("*********************************"));
  Serial.println(F(" Proyecto monitorCO2"));
  Serial.println(F(__DATE__  " " __TIME__));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println(F("(C) 2021, IER-UNAM"));
  Serial.println(F("(C) 2021, UNAM"));
  Serial.println(F("*********************************"));
  Serial.println();
#endif

#ifdef DEBUG
  Serial.println("Init display...");
#endif
  oled.begin(0x3C, true); // check error condition?
  oled.clearDisplay();
  oled.drawBitmap(0,0,splash_IER_64x128,64,128,FG_COLOR);
  oled.display();
  oled.setRotation(1);
  oled.setTextSize(1);
  oled.setTextColor(FG_COLOR);
  delay(LONG_DELAY);

  if(!scd30.begin()) {
#ifdef DEBUG
    Serial.println(F("Init scd30 failed!"));
#endif
  }
  delay(SHORT_DELAY);
  
  if(scd30.getMeasurementInterval()!=MEASUREMENT_INTERVAL) {
#ifdef DEBUG
    Serial.printf(PSTR("Interval=%d\n"), MEASUREMENT_INTERVAL);
#endif
    delay(SHORT_DELAY);
    if(!scd30.setMeasurementInterval(MEASUREMENT_INTERVAL)) ;
  }

  if(!scd30.selfCalibrationEnabled()) {
    delay(SHORT_DELAY);
    if(!scd30. selfCalibrationEnabled(true)) {
#ifdef DEBUG
      Serial.println("ASC enabled...");
#endif
    }
  }

#ifdef DEBUG
  Serial.println("leaving setup()");
#endif
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  if (scd30.dataReady()) {
    digitalWrite(LED_BUILTIN, HIGH);

    oled.clearDisplay();

    if (!scd30.read()){
#ifdef DEBUG
      Serial.println("Read Error!");
#endif
      oled.setCursor(0,0);
      oled.println("Read Error!");
    }

    String value;
    for(value = String(scd30.CO2, 0); value.length()<4; value=" " + value);
    oled.setFont(&FreeSansBold24pt7b);
    oled.setCursor(0,44);
    oled.print(value);
    oled.setFont();
    oled.setCursor(108,20);
    oled.print("CO2");
    oled.setCursor(108, 28);
    oled.print("ppm");

    float measuredvbat = 3.3 * analogRead(VBATPIN) / 512;
  
    oled.setCursor(0,56);
    oled.print(measuredvbat, 1);
    oled.print("V ");
    oled.print(scd30.temperature, 1);
    oled.print("C RH=");
    oled.print(scd30.relative_humidity, 0);
    oled.print("%");

    oled.display();
    digitalWrite(LED_BUILTIN, LOW);
  } else 
#ifdef DEBUG
  delay(MEASUREMENT_INTERVAL);
#else
  Watchdog.sleep(MEASUREMENT_INTERVAL);
#endif
}
