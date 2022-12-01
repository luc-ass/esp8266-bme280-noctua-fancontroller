#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define FAN_PIN 14               // Set GPIO FAN_PIN (GPIO 14 = D5)

/*Put your SSID & Password*/
const char* ssid = "NetworkName";         // Enter SSID here
const char* password = "SecretPassword";  // Enter Password here

float TARGET_TEMPERATURE = 30;  // Target temperature in °C
int PWM_FANSPEED = 128;         // Range 0 - 255, start with ~ 50 %
int PWM_STEPSIZE = 1;
float temperature;
float previous_temperature;     // store previous temperature

Adafruit_BME280 bme;

ESP8266WebServer server(80); 

void setup() {
  // set baudrate for serial monitor
  Serial.begin(115200);
  delay(100);

  Serial.print("Connecting to ");
  Serial.print(ssid);

  // set FAN_PIN to OUTPUT
  pinMode(FAN_PIN,OUTPUT);

  // initialize BME280
  bme.begin(0x76);

  // connect to your local WiFi network
  WiFi.begin(ssid, password);

  // check if board is connected to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  Serial.println((String)"Target temperature: " + TARGET_TEMPERATURE + "°C");
  Serial.println((String)"Initial fan speed: " + PWM_FANSPEED);
    
  delay(5000);
}

void loop() {
  server.handleClient();
  
  temperature = bme.readTemperature();
  Serial.print((String)"Temperature: " + temperature);

  // temperature too high and rising
  if (temperature > TARGET_TEMPERATURE && temperature >= previous_temperature) {
    Serial.print((String)", Temperature higher than " + TARGET_TEMPERATURE + " °C and rising, ");
    PWM_FANSPEED = PWM_FANSPEED + PWM_STEPSIZE;  // increase fan speed
    PWM_FANSPEED = min(PWM_FANSPEED, 255);       // select smaller value to ensure it
                                                 // never goes above 255 (invalid)
    
  }
  // temperature too high but falling
  else if (temperature > TARGET_TEMPERATURE && temperature < previous_temperature) {
    Serial.print((String)", Temperature higher than " + TARGET_TEMPERATURE + " °C but falling, ");
  }
  // temperautre just right
  else if (temperature == TARGET_TEMPERATURE) {
    Serial.print((String)", Temperature exactly " + TARGET_TEMPERATURE + " °C, ");
  }
  // temperature too low but rising
  else if (temperature < TARGET_TEMPERATURE && temperature > previous_temperature) {
    Serial.print((String)", Temperature lower than " + TARGET_TEMPERATURE + " °C but rising, ");
  }
  // temperature too low and falling
  else {
    Serial.print((String)", Temperature lower than " + TARGET_TEMPERATURE + " °C, ");
    PWM_FANSPEED = PWM_FANSPEED - PWM_STEPSIZE;  // lower fan speed
    PWM_FANSPEED = max(PWM_FANSPEED, 51);        // select larger value to ensure it never
                                                 // goes below 0 (invalid), 51 is now minimum
                                                 // as Noctuas lower limit is 20%
  }

  Serial.println((String)"Fan speed: " + PWM_FANSPEED);
  
  //PWM Value varries from 0 to 255 
  analogWrite(FAN_PIN,PWM_FANSPEED);

  previous_temperature = temperature;
  
  // increased speed to 1/s after optimizing logic about falling or rising temperature
  delay(1000);
}

void handle_OnConnect() {
  temperature = bme.readTemperature();
  server.send(200, "text/html", SendHTML(temperature)); 
  Serial.println((String)"Temperatur: " + temperature + "°C; Fan speed: " + PWM_FANSPEED);
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"5\">\n";
  ptr +="<title>Serverschrank Temperatur</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>Serverschrank Temperatur</h1>\n";
  ptr +="<p>Target temperature: ";
  ptr +=TARGET_TEMPERATURE;
  ptr +="&deg;C</p>";
  ptr +="<p>Temperature: <span id=\"temperature\">";
  ptr +=temperature;
  ptr +="</span> &deg;C</p>";
  ptr +="<p>Fan speed: <span id=\"fanspeed\">";
  ptr +=PWM_FANSPEED;
  ptr +="</speed>/255</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
