/****************************************
 * Sensor SEN0220 0-50K CO2 ppm            
 *   Serial1 requires Arduino Leonardo
 *   or similar board
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#define DELAY 4096

unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 

typedef struct {
  byte ff, cmd, hi, lo, fill[4], crc;
} data_t;

int sen0220(void) {
  register union {
    struct { byte lo, hi; };
    word w;
  } retval;
  data_t data;
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

void setup() {

  Serial.begin(38400);
  while (!Serial) {

  }
  Serial.println(F("*****************************************"));
  Serial.println(F("Test program for CO2 Sensor SEN0220 "));
  Serial.println(F("(CC) Aztec Eagle Turbo, turbo@ier.unam.mx"));
  Serial.println(F("*****************************************"));
  Serial.println();

  Serial.print(F("Initializing..."));
  Serial1.begin(9600);
  while (!Serial1) {

  }
  Serial.println(F("Done!"));
}

void loop() {
  unsigned long uptime = millis()/1000;
  word co2sen0220 = sen0220();

  Serial.print(uptime);
  Serial.print(" ");
  Serial.print(co2sen0220);
  Serial.println();
  delay(DELAY);
}
