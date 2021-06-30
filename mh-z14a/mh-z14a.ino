/*
  CO2-all-sensors

    Requires Arduino Yún 
    Requires Linux script to send data to MQTT broker

  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>

  Quick&Dirty copy-paste samples for CO2 sensors:
  
  * sen0219 / mh-z14a
  * sen0220
  * t3022

*/

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 8129

/****************************************
 * Sensor MH-Z14A
 *   Serial1 requires Arduino Leonardo
 *   or similar board
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/

unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 

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

/*********************
 * Main program here!
 *********************/
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
  Serial.println(F("Test program for CO2 Sensor SEN0219 "));
  Serial.println(F(" Analog and Digital interfaces\n"));
  Serial.println(F("(C) Aztec Eagle Turbo, turbo@ier.unam.mx"));
  Serial.println(F("*****************************************\n"));

  Serial.print(F("Initializing..."));
  Serial1.begin(9600);
  while (!Serial1) {
    digitalWrite(LED_BUILTIN, led=!led);
    delay(SHORT_DELAY);  
    }
  Serial.println(F("\nDone!"));
}

void loop() {
  unsigned long uptime = millis()/1000;

  digitalWrite(LED_BUILTIN, led=!led);

  int co2sen0219analog = sen0219_analog();
  int co2sen0219digital = sen0219_digital();

  Serial.print(uptime);
  Serial.print(" ");
  Serial.print(co2sen0219analog);
  Serial.print(" ");
  Serial.print(co2sen0219digital);
  Serial.println();
  delay(DELAY);
}
