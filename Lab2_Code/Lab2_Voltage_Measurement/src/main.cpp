#include <Arduino.h>


const int pin_vout1 = A0; // 对应 J2
const int pin_vout2 = A1; // 对应 J3

void setup() {
 
  Serial.begin(115200);
  
 
  pinMode(pin_vout1, INPUT);
  pinMode(pin_vout2, INPUT);
  

  analogReadResolution(12);
}

void loop() {
  
  int adc1 = analogRead(pin_vout1);
  int adc2 = analogRead(pin_vout2);
  
 
  float voltage1 = (adc1 / 4095.0) * 3.3;
  float voltage2 = (adc2 / 4095.0) * 3.3;
  
 
  Serial.print("VOUT1 (J2) ADC: ");
  Serial.print(adc1);
  Serial.print(" | Voltage: ");
  Serial.print(voltage1);
  Serial.println(" V");
  
  Serial.print("VOUT2 (J3) ADC: ");
  Serial.print(adc2);
  Serial.print(" | Voltage: ");
  Serial.print(voltage2);
  Serial.println(" V");
  
  Serial.println("-----------------------");
  
  delay(1000);
}