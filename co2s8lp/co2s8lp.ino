/****************************************
 * Senseair S8 LP 0-2K CO2 ppm
 *   Requieres Arduino Leonardo UART
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#include <TM1637Display.h>

// #define DEBUG

#define SHORTDELAY 128
#define DELAY 1024
#define LONGDELAY 4096

#define CLK 2
#define DIO 3

TM1637Display display(CLK, DIO);

boolean led = false;

static const byte CMD_STATUS[] = {0xfe, 0x04, 0x00, 0x00, 0x00, 0x01, 0x25, 0xc5};
static const byte CMD_CO2READ[] = {0xfe, 0x04, 0x00, 0x03, 0x00, 0x01, 0xd5, 0xc5};

typedef struct {
  byte addr,
   function_code,
   count,
   hi,
   lo,
   crc_hi,
   crc_lo;
} data7_t;

void sendData(const void* p, size_t n) {
  Serial1.write((char*)p,n);
}

void receiveData(const void* p, size_t n) {
  if (Serial1.available()>0)
    Serial1.readBytes((char*)p,n);
}

typedef union {
    struct {
      byte lo,hi;
    };
    word w;
  } retval_t;

#define S8_WAIT_DELAY 100

int s8status(void) {
  retval_t retval;
  data7_t data;
  
  sendData(CMD_STATUS, sizeof(CMD_STATUS));
  delay(S8_WAIT_DELAY);
  receiveData(&data, sizeof(data));

  if((data.addr != 0xfe) && (data.function_code != 0x04)) return -1;
  else {
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
}

int s8co2read(void) {
  retval_t retval;
  data7_t data;
  
  sendData(CMD_CO2READ, sizeof(CMD_CO2READ));
  delay(S8_WAIT_DELAY);
  receiveData(&data, sizeof(data));

  if((data.addr != 0xfe) && (data.function_code != 0x04)) return -1;
  else {
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
}

void setup(){  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, led=!led);
  
  display.setBrightness(0x07);
  display.showNumberDec(8888);
  delay(LONGDELAY);

  Serial.begin(38400);
#ifdef DEBUG
  while (!Serial) {
    digitalWrite(LED_BUILTIN, led=!led);
    delay(SHORTDELAY);
  }
#endif
  Serial.println(F("\n****************************************"));
  Serial.println(F("Test program for S8 LP CO2 Sensor Series"));
  Serial.println(F("(C) Aztec Eagle Turbo, turbo@ier.unam.mx"));
  Serial.println(F("****************************************\n"));

  Serial1.begin(9600);
  while (!Serial1) {
    digitalWrite(LED_BUILTIN, led=!led);
    delay(SHORTDELAY);
  }
  
  Serial.println(F("Initializing..."));
  while(1) { 
    digitalWrite(LED_BUILTIN, led=!led);
    word st=s8status();
    display.showNumberHexEx(st, 0, true);
    Serial.print(F("ST=0x"));
    Serial.println(st, HEX);
    if(st==0) break;
    delay(DELAY);
  }
}

void loop(){
  unsigned long uptime = millis()/1000;
  digitalWrite(LED_BUILTIN, led=!led);

  int co2ppm = s8co2read();

  display.showNumberDec(co2ppm);

  Serial.print(uptime);
  Serial.print(" ");
  Serial.print(co2ppm);
  Serial.println();

  delay(LONGDELAY);
}
