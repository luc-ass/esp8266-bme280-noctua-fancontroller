#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define FAN_PIN 14               // Set GPIO FAN_PIN (GPIO 14 = D5)

float TARGET_TEMPERATURE = 30;  // Target temperature in °C
int PWM_FANSPEED = 512;         // Range 0 - 0123, start with ~ 50 %
float temperature;

Adafruit_BME280 bme;

void setup() {
  // set baudrate for serial monitor
  Serial.begin(115200);

  // set FAN_PIN to OUTPUT
  pinMode(FAN_PIN,OUTPUT);

  // initialize BME280
  bme.begin(0x76);

  Serial.println("System started with the following settings:");
  Serial.println((String)"Target temperature: " + TARGET_TEMPERATURE + "°C");
  Serial.println((String)"Initial fan speed: " + PWM_FANSPEED);
    
  delay(5000);
}

void loop() {
  temperature = bme.readTemperature();
  Serial.println((String)"Temperature: " + temperature);

  if (temperature > TARGET_TEMPERATURE) {
    Serial.println((String)"Temperature higher than " + TARGET_TEMPERATURE + " °C");
    PWM_FANSPEED = PWM_FANSPEED + 16;       // increase fan speed
    PWM_FANSPEED = min(PWM_FANSPEED, 1023); // select smaller value to ensure it
                                            // never goes above 1023 (invalid)
    
  }
  else if (temperature == TARGET_TEMPERATURE) {
    Serial.println((String)"Temperature exactly " + TARGET_TEMPERATURE + " °C");
  }
  else {
    Serial.println((String)"Temperature lower than " + TARGET_TEMPERATURE + " °C");
    PWM_FANSPEED = PWM_FANSPEED - 16;       // lower fan speed
    PWM_FANSPEED = max(PWM_FANSPEED, 0);    // select larger value to ensure it never
                                            // goes below 0 (invalid)
  }

  Serial.println((String)"Fan speed: " + PWM_FANSPEED);
  
  //PWM Value varries from 0 to 1023 
  analogWrite(FAN_PIN,PWM_FANSPEED);
  delay(3000); 
}