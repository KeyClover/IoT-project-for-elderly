// template name and ID
#define BLYNK_TEMPLATE_ID "TMPL6TRDNp9t8"
#define BLYNK_TEMPLATE_NAME "Object Detection and Fall Detection Systems"
#define BLYNK_AUTH_TOKEN "ipr1suYBAlrjVOKm6wvjXgwlcPTh7ZyA"

// Include Libraries
#include <Arduino.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <DHT.h>

// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "IT_lab 2.4G";
char pass[] = "csr20219";

#define trigPin D1
#define echoPin D2
#define buzzer D0
#define LED1 D3
#define LED2 D4
#define DHT_PIN D5
#define LED3 D6
#define LED4 D7

DHT dht(DHT_PIN, DHT11);

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034

long duration;
float distanceCm; // global variable

unsigned long startTime = 0;
const unsigned long holdTime = 10000;  // 10 seconds in milliseconds
const int minDistance = 780;  // Minimum distance for the range 
const int maxDistance = 800;  // Maximum distance for the range 

void sendSensorData() {
  // ultrasonic
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_VELOCITY/2;

  Blynk.virtualWrite(V0, distanceCm);
  
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  // DHT sensor
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V3, t); // Send temperature to Blynk
    Blynk.virtualWrite(V4, h); // Send humidity to Blynk
  }
}

BlynkTimer timer;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(500L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
  objectAlert(); // call the object alert function when the object is closer than 30 centimeters
  fallDetectedAlert();
  hotTemp(); // when the temperature is too hot
}

// action function
void objectAlert() { // ultrasonic
  if(distanceCm <= 30) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    delay(500); // delay 0.3 second
    digitalWrite(buzzer, LOW);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    delay(200);
  }
}

void fallDetectedAlert() {
  if (distanceCm >= minDistance && distanceCm <= maxDistance) {
    if (startTime == 0) {
      startTime = millis();  // Start the timer
    }
  // Check if the timer has reached the hold time (10 seconds)
    if (millis() - startTime >= holdTime) {
      Blynk.logEvent("fall_event");
      // The distance has remained constant within the range for 10 seconds, trigger the buzzer
      digitalWrite(buzzer, HIGH);
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      delay(2000);  // Buzzer on for 1 second
      // startTime = 0;  // Reset the timer
    }
  } else {
    // Reset the timer when the distance is outside the range
    startTime = 0;
  }
}

void hotTemp () { // alert when the temperature is too hot!
  if (dht.readTemperature() > 40.0) {
    Blynk.logEvent("heatstroke_cautions");
  }
}

// Night mode control
BLYNK_WRITE(V5) {
  if(param.asInt() == 1)
  {
    // On
    digitalWrite(LED3,HIGH);
    digitalWrite(LED4, HIGH);
  } else {
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
  }
}