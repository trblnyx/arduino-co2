/*
  Winsen MH-Z14A

    Requires Arduino Leonardo 

  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>

  * sen0219 / mh-z14a

  ANALOG, UART & PWM outputs
*/

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 8129

// WDT Interrupt Driver
volatile short __wdt_counter = 0;

ISR(WDT_vect) {
  ++__wdt_counter;
}

/****************************************
 * Sensor MH-Z14A
 *   Serial1 requires Arduino Leonardo
 *   or similar board
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/

unsigned char hexdata[] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 

typedef struct {
  byte ff, cmd, hi, lo, fill[4], crc;
} data_sen0220_t;

int sen0219_digital(void) {
  register union {
    struct { byte lo, hi; };
    word w;
  } retval;
  data_sen0220_t data;
  Serial1.write(hexdata,9);
  delay(500);
  if (Serial1.available()>0) { 
    Serial1.readBytes((char*)&data, 9);
    if((data.ff != 0xff) && (data.cmd != 0x86)) return -1;
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
  else return -1;    
}

/****************************************
 * Sensor SEN0219 0-5K CO2 ppm            
 *   Requieres AREF=3.3V
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#define PIN_SEN0219 A0

int sen0219_analog(void) {
  int sensorValue = analogRead(PIN_SEN0219);
  float voltage = sensorValue*(3300/1024.0);
  if(voltage<400) return -1;
  else {
    int voltage_diference=voltage-400;
    return int(voltage_diference*50.0/16.0);
  }
}

/****************************************
 * Sensor MH-Z14A PWM ISR
 *   INT0 requires Arduino Leonardo
 *   or similar board
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#define PIN 2

typedef struct {
  unsigned long start_hi, start_lo, width;
} pwm_t;

volatile pwm_t *p;

void PIN_ISR() {
  unsigned long now = micros();
  int value = digitalRead(PIN);
  if(value == HIGH) 
    p->start_hi = now;
  else
    p->width = (p->start_lo = now) - p->start_hi;
}

int sen0219_pwm(void) {
  return map(p->width/1000, 2, 1002, 0, 2000);
}

/*********************
 * Main program here!
 *********************/
unsigned char SET_RANGE_2K[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x07,0xD0,0x8F};

boolean led = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, led=!led);
  analogReference(EXTERNAL);

  Serial.begin(38400);
  while (!Serial) {
    digitalWrite(LED_BUILTIN, led=!led);
    delay(SHORT_DELAY);  
  }

  Serial.println(F("\n*****************************************"));
  Serial.println(F("Test program for CO2 Sensor MH-Z14A "));
  Serial.println(F(" Analog, UART & PWM interfaces\n"));
  Serial.println(F("(C) Aztec Eagle Turbo, turbo@ier.unam.mx"));
  Serial.println(F("*****************************************\n"));

  Serial.print(F("Serial1..."));
  Serial1.begin(9600);
  while (!Serial1) {
    digitalWrite(LED_BUILTIN, led=!led);
    delay(SHORT_DELAY);  
    }
  Serial.println(F("\nDone!"));

  Serial.print(F("Setting range to 2000ppm..."));
  Serial1.write(SET_RANGE_2K, sizeof(SET_RANGE_2K));
  Serial.println(F("\nDone!"));

  Serial.print(F("Setting INT0 Interrupt..."));
  p=new pwm_t;
  p->start_hi=p->start_lo=p->width=0;
  attachInterrupt(digitalPinToInterrupt(PIN), PIN_ISR, CHANGE);
  Serial.println(F("\nDone!"));

  Serial.println(F("Setting WDT Interrupt"));
// WDT Timer Setup
  MCUSR &= ~(1<<WDRF);
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = 1<<WDP1 | 1<<WDP2; // 1s
  WDTCSR |= _BV(WDIE);
  Serial.println(F("Done!"));  
}

int co2sen0219analog, 
    co2sen0219digital, 
    co2sen0219pwm;

byte interval = 0;

void loop() {
  if(__wdt_counter==0) return;
  --__wdt_counter; 

  digitalWrite(LED_BUILTIN, led=!led);

  switch(interval++) {
    case 0:
      co2sen0219analog = sen0219_analog();
      break;
    case 1:
      co2sen0219digital = sen0219_digital();
      break;
    case 2:
      co2sen0219pwm = sen0219_pwm();
      break;
    case 3:
      interval=0;
      unsigned long uptime = millis()/1000;
      Serial.print(uptime);
      Serial.print(" ");
      Serial.print(co2sen0219analog);
      Serial.print(" ");
      Serial.print(co2sen0219digital);
      Serial.print(" ");
      Serial.print(co2sen0219pwm);
      Serial.println();
      break;      
  }
}
