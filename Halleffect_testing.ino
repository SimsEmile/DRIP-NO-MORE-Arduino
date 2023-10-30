#include <Arduino.h>

int LED_pin = 2;
int magnet_pin = 38;

void setup() {
  pinMode(magnet_pin, INPUT);
  pinMode(LED_pin, OUTPUT);
}

void loop() {
  int currentHallState = digitalRead(magnet_pin);
  if (currentHallState == LOW){
    digitalWrite(LED_pin, HIGH);
  } else {
    digitalWrite(LED_pin, LOW);
    delay(1000);
  }
  
    // Reset the timer
    
}
