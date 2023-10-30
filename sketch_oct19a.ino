#include <Arduino.h>
int LED_pin = 2;
void setup() {
  pinMode(LED_pin, OUTPUT);

}

void loop() {
  for (int i = 0; i < 20; i++){
    digitalWrite(LED_pin, HIGH);
    delay(500);
    digitalWrite(LED_pin, LOW);
    delay(500);
  }
}
