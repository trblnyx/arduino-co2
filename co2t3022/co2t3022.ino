/****************************************
 * Sensor T3022 0-5K CO2 ppm            
 *   i2c addr = 0x15
 *   cmdformat = modbus
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#include <Wire.h>
#include <TM1637Display.h>

#define DELAY 4096
#define WAITINIT 120000L

#define I2C_ADDR 0x15

#define CLK 5
#define DIO 4

TM1637Display display(CLK, DIO);

boolean led = false;

static const byte CMD_FWREV[] = {0x04, 0x13, 0x89, 0x00, 0x01};

typedef struct {
  byte function_code;
  byte count;
  word revision;
} data_fwrev_t;

data_fwrev_t data_fwrev;

static const byte CMD_STATUS[] = {0x04, 0x13, 0x8A, 0x00, 0x01};

typedef struct {
  byte function_code;
  byte count;
  word status;
} data_status_t;

data_status_t data_status;

static const byte CMD_CO2PPM[] = {0x04, 0x13, 0x8B, 0x00, 0x01};

typedef struct {
  byte function_code;
  byte count;
  byte hi;
  byte lo;
} data_co2ppm_t;

data_co2ppm_t data_co2ppm;

typedef struct {
  byte fc, bc, hi, lo;
} data_t;

void sendData(void* p, size_t n) {
  byte i;
  for(i=0; i<n; ++i)
    Wire.write(((byte*)p)[i]);
}

void receiveData(void* p, size_t n) {
  byte i;
  for(i=0; i<n; ++i)
    ((byte*)p)[i]=Wire.read();
}

word t3022(void* p) {
  union {
    struct {
      byte lo,hi;
    };
    word w;
  } retval;
  data_t data;
  Wire.beginTransmission(I2C_ADDR);
  sendData(p, 5);
  Wire.endTransmission();
  delay(50); // T67xx CO2 sensor module Application note
  Wire.requestFrom(I2C_ADDR, sizeof(data_t));
  receiveData(&data, sizeof(data_t));
  if( (data.fc==4) && (data.bc==2)) {
    retval.hi=data.hi; retval.lo=data.lo;
    return retval.w;
  } else return -1;
}

void setup(){  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, led=!led);
  
  display.setBrightness(0x07);
  display.showNumberDec(8888);

  Serial.begin(38400);
  Serial.println(F("*****************************************"));
  Serial.println(F("Test program for T3022 CO2 Sensor Series"));
  Serial.println(F("(CC) Aztec Eagle Turbo, turbo@ier.unam.mx"));
  Serial.println(F("*****************************************"));
  Serial.println();
  Wire.begin();

  delay(DELAY);

  Serial.println(F("Detecting..."));
  Serial.print(F("FWREV=0x"));
  word fwrev=t3022(CMD_FWREV);
  Serial.println(fwrev, HEX);
  
  Serial.println(F("Initializing..."));
  word i;
  for(i=1+WAITINIT/DELAY; i>0; --i) {
    digitalWrite(LED_BUILTIN, led=!led);
    display.showNumberDec(i, true);
    word st=t3022(CMD_STATUS);
    Serial.print(F("ST=0x"));
    Serial.println(st, HEX);
    if(st==0) break;
    delay(DELAY);
  }
}

void loop(){
  unsigned long uptime = millis()/1000;
  digitalWrite(LED_BUILTIN, led=!led);

/*
// Get Firmware Revision
  Serial.println(F("Sending cmd_fwrev..."));

  Wire.beginTransmission(I2C_ADDR);
  sendData(CMD_FWREV, sizeof(CMD_FWREV));
  Wire.endTransmission();

  Serial.println(F("Waiting response..."));

  Wire.requestFrom(I2C_ADDR, sizeof(data_fwrev));
  receiveData(&data_fwrev, sizeof(data_fwrev));

  Serial.print("FC=0x");
  Serial.print(data_fwrev.function_code);
  Serial.print(" BC=0x");
  Serial.print(data_fwrev.count);
  Serial.print(" REV=0x");
  Serial.println(data_fwrev.revision, HEX);
  Serial.println();
  
  delay(1024);

// Get Status  
  Serial.println(F("Sending cmd_status..."));

  Wire.beginTransmission(I2C_ADDR);
  sendData(CMD_STATUS, sizeof(CMD_STATUS));
  Wire.endTransmission();

  Serial.println(F("Waiting response..."));

  Wire.requestFrom(I2C_ADDR, sizeof(data_status));
  receiveData(&data_status, sizeof(data_status));

  Serial.print("FC=0x");
  Serial.print(data_status.function_code);
  Serial.print(" BC=0x");
  Serial.print(data_status.count);
  Serial.print(" ST=0x");
  Serial.println(data_status.status, HEX);
  Serial.println();

  delay(1024);

// Get CO2PPM
  Serial.println(F("Sending cmd_co2ppm..."));

  Wire.beginTransmission(I2C_ADDR);
  sendData(CMD_CO2PPM, sizeof(CMD_CO2PPM));
  Wire.endTransmission();

  Serial.println(F("Waiting response..."));

  Wire.requestFrom(I2C_ADDR, sizeof(data_co2ppm));
  receiveData(&data_co2ppm, sizeof(data_co2ppm));

  Serial.print("FC=0x");
  Serial.print(data_co2ppm.function_code);
  Serial.print(" BC=0x");
  Serial.print(data_co2ppm.count);
  Serial.print(" CO2=0x");
  Serial.println((256*data_co2ppm.hi+data_co2ppm.lo), HEX);
  Serial.println();
  
  delay(1024);
*/
  word co2ppm = t3022(CMD_CO2PPM);

  display.showNumberDec(co2ppm);

  Serial.print(uptime);
  Serial.print(" ");
  Serial.print(co2ppm);
  Serial.println();

  delay(DELAY);
}
