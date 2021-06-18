/***************************************************
 * Infrared CO2 Sensor0-5000ppm
 * ****************************************************
 * This example The sensors detect CO2
 *
 * @author lg.gang(lg.gang@qq.com)
 * @version  V1.0
 * @date  2016-7-6
 *
 * GNU Lesser General Public License.
 * See <http://www.gnu.org/licenses/> for details.
 * All above must be included in any redistribution
 *
 * Modified by <hdcg@ier.unam.mx> to use SerialPlotter
 * Added Blinking LED 
 * AREF connected to 3V3 
 * Added a 4 digit 7-segment LED display
 * 
 * ****************************************************/
int sensorIn = A0;

boolean led = false;

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, led=!led);
  
  Serial.begin(38400);
  // Connect AREF pin to 3.3V pin 
  analogReference(EXTERNAL);
  Serial.println("sensorValue voltage co2ppm");
}

void loop(){
  digitalWrite(LED_BUILTIN, led=!led);

  //Read voltage
  int sensorValue = analogRead(sensorIn);

  // The analog signal is converted to a voltage
  float voltage = sensorValue*(3300/1024.0);
  if(voltage == 0)
  {
    Serial.println("Fault");
  }
  else if(voltage < 400)
  {
    Serial.println("preheating");
  }
  else
  {
    int voltage_diference=voltage-400;
    float concentration=voltage_diference*50.0/16.0;
    // Print sensorValue
    Serial.print(sensorValue);
    // Print Voltage
    Serial.print(" ");
    Serial.print(voltage);
    // Serial.println("mv");
    //Print CO2 concentration
    Serial.print(" ");
    Serial.println(concentration);
    //Serial.println("ppm");
  }
  delay(1024);
}
