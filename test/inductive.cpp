#include <Arduino.h>
 #include <Wire.h>
const int inductiveSensorPin = 34;  // ESP32 input pin


void setup() {
  // Set the inductive sensor pin as an input
  pinMode(inductiveSensorPin, INPUT);
  // Initialize serial communication at 9600 bits per second
  Serial.begin(115200);
}

void loop() {
  // Read the state of the inductive sensor output
  int sensorState = digitalRead(inductiveSensorPin);
  
  if (sensorState == 0)
  {
    // The sensor is triggered
    Serial.println("metal triggered!");
  }
  else
  {
    // The sensor is not triggered
    Serial.println("metal not triggered.");
  }
  // Print the sensor state to the Serial Monitor
  Serial.println(sensorState);
  
  // Add a delay for stability
  delay(100);
}
