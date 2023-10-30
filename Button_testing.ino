#include <Arduino.h>

const int buttonPin = 4; // Replace with the appropriate pin number
int LED_pin = 2;

void setup() {
  pinMode(buttonPin, INPUT); // Configure the pin as an input
  pinMode(LED_pin, OUTPUT);
}

void loop() {
  if (digitalRead(buttonPin) == HIGH){
    digitalWrite(LED_pin, HIGH);
    Serial.write("cool");
  }
  if (digitalRead(buttonPin) == LOW){
    digitalWrite(LED_pin, LOW);
  }
}