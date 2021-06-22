/*
  CO2-all-sensors

    Requires Arduino Uno WiFi Rev.2 board

  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>

  Quick&Dirty copy-paste samples for CO2 sensors:
  
  * sen0219
  * sen0220
  * t3022

*/

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ESP8266_ESP12)
  #include <ESP8266WiFi.h>
#endif
#include <ArduinoHttpClient.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// To connect with SSL/TLS:
//   Change WiFiClient to WiFiSSLClient.

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SECRET_SERVER, SECRET_PORT);

const long interval = 8192;
unsigned long previousMillis = 0;

#define DELAY 1024
/****************************************
 * Sensor T3022 0-5K CO2 ppm            
 *   i2c addr = 0x15
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#include <Wire.h>

#define I2C_ADDR (byte)0x15

static const byte CMD_FWREV[] = {0x04, 0x13, 0x89, 0x00, 0x01};
static const byte CMD_STATUS[] = {0x04, 0x13, 0x8A, 0x00, 0x01};
static const byte CMD_CO2PPM[] = {0x04, 0x13, 0x8B, 0x00, 0x01};

typedef struct {
  byte fc, bc, hi, lo;
} data_t3022_t;

void sendData(const void* p, size_t n) {
  byte i;
  for(i=0; i<n; ++i)
    Wire.write(((byte*)p)[i]);
}

void receiveData(const void* p, size_t n) {
  byte i;
  for(i=0; i<n; ++i)
    ((byte*)p)[i]=Wire.read();
}

word t3022(const void* p) {
  union {
    struct {
      byte lo,hi;
    };
    word w;
  } retval;
  data_t3022_t data;
  Wire.beginTransmission(I2C_ADDR);
  sendData(p, 5);
  Wire.endTransmission();
  delay(50); // T67xx CO2 sensor module Application note
  Wire.requestFrom(I2C_ADDR, sizeof(data));
  receiveData(&data, sizeof(data));
  if( (data.fc==4) && (data.bc==2)) {
    retval.hi=data.hi; retval.lo=data.lo;
    return retval.w;
  } else return -1;
}

/****************************************
 * Sensor SEN0220 0-50K CO2 ppm            
 *   Serial1 requires Arduino Leonardo
 *   or similar board
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/

unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 

typedef struct {
  byte ff, cmd, hi, lo, fill[4], crc;
} data_sen0220_t;

int sen0220(void) {
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

int sen0219(void) {
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

  //Initialize serial and wait for port to open:
  Serial.begin(38400);
  while (!Serial) {
    digitalWrite(LED_BUILTIN, led=!led);
    delay(DELAY);
  }
  Serial.println();
  Serial.println(F("***************************"));
  Serial.println(F(" CO2 all sensors"));
  Serial.println(F(" * sen0219\n * sen0220\n * t3022"));
  Serial.println(F(" (C) 2021 hdcg@ier.unam.mx"));
  Serial.println(F("***************************"));
  Serial.println();

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    digitalWrite(LED_BUILTIN, led=!led);
    Serial.print(".");
    delay(DELAY);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print(F("Init[sen0220]..."));
  Serial1.begin(9600);
  while (!Serial1) {
    digitalWrite(LED_BUILTIN, led=!led);
    delay(DELAY);
  }
  Serial.println(F("\nDone!")); 

  Wire.begin();

  Serial.println(F("Detect[t3022]..."));
  Serial.print(F("FWREV=0x"));
  int fwrev=t3022(CMD_FWREV);
  Serial.println(fwrev, HEX);
  
  Serial.println(F("Init[t3022]..."));
  while(1) {
    digitalWrite(LED_BUILTIN, led=!led);
    word st=t3022(CMD_STATUS);
    Serial.print(F("ST=0x"));
    Serial.println(st, HEX);
    if(st==0) break;
    delay(DELAY);
  }
}

void loop() {
  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    unsigned long uptime = (previousMillis = currentMillis) / 1000;

    digitalWrite(LED_BUILTIN, led=!led);

    int co2sen0219 = sen0219();
    
    int co2sen0220 = sen0220();

    int co2t3022 = t3022(CMD_CO2PPM);
 
    String payload = "{";
    payload += "'uptime':";
    payload += uptime;
    payload += ",'sen0219':";
    payload += co2sen0219;
    payload += ",'sen0220':";
    payload += co2sen0220;
    payload += ",'t3022':";
    payload += co2t3022;
    payload += "}";

    Serial.println("POST... " + payload);

    digitalWrite(LED_BUILTIN, led=!led);
    httpClient.post(SECRET_TOPIC, "application/json", payload);

    int statusCode = httpClient.responseStatusCode();  
    String response = httpClient.responseBody();
    Serial.print("Status code: ");
    Serial.print(statusCode);
    Serial.print(" ");
    Serial.println(response);
    digitalWrite(LED_BUILTIN, led=!led);
  }
}
