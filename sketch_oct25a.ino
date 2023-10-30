#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <ArduinoJson.h>

const char* ssid = "Dripping";
const char* password = "No-more2";
// Declare a public array with first details
int myArray[6];

const int EN_pin = 1;
//hall effect pin to get data from it
const int hallEffectPin = 5;
//pin for receiving buttonpressed data
const int buttonPin = 7; // Replace with the appropriate pin number
//pin for receiving water flow data
const int WaterFlowPin = 3;
//calibration factor advised by manufacturer
const float calibrationFactor = 9.5;

WebServer server(80);
WiFiClient client;

//main setup, used to setup the pinModes for the different sensors and valve, as well as the connection to the app
void setup() {
  //Default settings of the arduino
  myArray[0] = 0;
  myArray[1] = 150;
  myArray[2] = 300;
  myArray[3] = 500;
  myArray[4] = 1000;
  myArray[5] = 2000;
  //Setup the pins
  pinMode(EN_pin, OUTPUT);
  pinMode(hallEffectPin, INPUT); // Configure the pin as an input
  pinMode(buttonPin, INPUT); // Configure the pin as an input
  pinMode(WaterFlowPin, INPUT);
  Serial.println("cool");
  IPAddress staticIP(192, 168, 1, 100); // Set your desired static IP address
  Serial.begin(115200);
  WiFi.softAP("ESP32");
  server.on("/data", HTTP_GET, handleData);

  server.begin();
  delay(2000);
  digitalWrite(EN_pin, HIGH);
  delay(1000);
  digitalWrite(EN_pin, LOW);
}
/*
  First, check if a user wishes to send data. If there is a client, check if there is data. If not, we check for a magnet count
  The magnet count is then sent to Buttonpressed, to check if the button has been pressed
*/
void loop() {
  server.handleClient(); // Call the CheckConnection function
  Buttonpressed();
}
/*
  Counts the number of magnets. The normal mode is considered 0, so goes up until 5.
*/
void countMagnets() {
  float magnetCount = 0;
  int lastHallState = digitalRead(hallEffectPin);
  unsigned long timerStarted = millis();
  while (millis()-timerStarted < 5000){
    int currentHallState = digitalRead(hallEffectPin);
    if (currentHallState != lastHallState) {
      // A change in the Hall Effect sensor state indicates a magnet was detected, participates for half the change.
      magnetCount = magnetCount + 0.5;
    lastHallState = currentHallState;
    }   
  }
  if (magnetCount == 0){
    normalvalve();
  }
  else{
    float waterflow = calibrate();
    long duration = ((myArray[int(floor(magnetCount))]*0.001)/(waterflow/60))*1000; //Amount of water needed (in ml), so divide by 1000 to get liters, divided by the water flow in L/s (reason for dividing by 60, cause minutes. Then, *1000 to get this in ms.)
    SolanoidValve(duration);
  }
}


/*
  Checks for 3 seconds if the Button has been pressed. If not, we go back to the loop.
*/
void Buttonpressed(){
  int buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH){
    countMagnets();
  }
}
/*
  analyzes the water flow rate for if the button has been pressed
*/
float calibrate(){
  unsigned long start = millis();
  int pulseCount = 0;
  while (millis() - start < 1000){
    if (digitalRead(WaterFlowPin) == HIGH) {
      pulseCount++;
    }
  }
  float flowRate = calculateFlowRate(pulseCount, calibrationFactor);
  return flowRate;
}

/*
  measure flow rate depending on pulsecount read in calibrate, used for the duration formula.
*/
float calculateFlowRate(int pulseCount, float calibrationFactor) {
  // Calculate the flow rate in liters per minute
  float flowRate = (pulseCount / calibrationFactor) * 60.0;
  return flowRate;
}

/*
  Function to open the valve for a specific duration (in ms, hence the *1000)
*/
void SolanoidValve(unsigned long duration) {
  long start_time = millis();
  digitalWrite(EN_pin, HIGH);
  while ((millis() - start_time < duration) && (digitalRead(buttonPin) == HIGH)){
  }
  digitalWrite(EN_pin, LOW);
  /*
  while (digitalRead(buttonpin) == HIGH){
  }
  */
  delay(1000);
}
/*
  If we are in the normal valve (magnetCount = 0), we have the normal valve, which while the button is active, will stay open, until it is closed.
*/
void normalvalve(){
  digitalWrite(EN_pin, HIGH);
  while (digitalRead(buttonPin) != LOW){
  }
  digitalWrite(EN_pin, LOW);
  delay(1000);
}





void handleData() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");

    // Create a JSON document with a capacity to hold 5 elements
    const size_t capacity = 5 * JSON_OBJECT_SIZE(1); // 1 is the size of each string value
    DynamicJsonDocument doc(capacity);

    // Deserialize the JSON data
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        server.send(400, "text/plain", "Bad JSON Data");
    } else {
        if (doc.is<JsonArray>()) {
            JsonArray array = doc.as<JsonArray>();

            // Create a list (an array) to hold the string values
            int values[5];
            //Create list to replace MyArray
            int newArray[6];
            newArray[0] = myArray[0];
            // Extract the string values from the JSON array
            for (int i = 0; i < array.size(); i++) {
                values[i] = array[i].as<int>();
                newArray[i+1] = values[i];
            }
            for (int i=0; i < 6; i++){
              myArray[i] = newArray[i];
            }

            // You can use them as needed
            // For example, values[0] contains the first string, values[1] contains the second, and so on

            server.send(200, "text/plain", "Data received and processed");
        } else {
            server.send(400, "text/plain", "Invalid JSON Data Format");
        }
    }
  } else {
      server.send(400, "text/plain", "Bad Request");
  }
}
