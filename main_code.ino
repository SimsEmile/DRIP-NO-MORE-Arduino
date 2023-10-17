#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ArduinoJson.h>

const char* ssid = "4Ipod";
const char* password = "Louis2012";
// Declare a public array
int myArray[] = {0, 150, 300, 500, 1000, 2000};
//solanoid pin for the valve
const int solenoidPin = 2;
//hall effect pin to get data from it
const int hallEffectPin = 38;
//pin for receiving buttonpressed data
const int buttonPin = 4; // Replace with the appropriate pin number
//pin for receiving water flow data
const int WaterFlowPin = 36;
//calibration factor advised by manufacturer
const float calibrationFactor = 9.5;

WiFiServer server(80);
WiFiClient client;

//main setup, used to setup the pinModes for the different sensors and valve, as well as the connection to the app
void setup() {
  //Setup the pins
  pinMode(solenoidPin, OUTPUT);
  pinMode(hallEffectPin, INPUT); // Configure the pin as an input
  pinMode(buttonPin, INPUT); // Configure the pin as an input
  pinMode(WaterFlowPin, INPUT);
  Serial.println("cool");


  Serial.begin(115200);
  delay(10);

  // Connect to Wi-Fi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}
/*
  First, check if a user wishes to send data. If there is a client, check if there is data. If not, we check for a magnet count
  The magnet count is then sent to Buttonpressed, to check if the button has been pressed
*/
void loop() {
  if (client) {
    CheckConnection(client); // Call the CheckConnection function
  }
  int setup = countMagnets();
  Buttonpressed(setup);
}
/*
  Counts the number of magnets. The normal mode is considered 0, so goes up until 5.
*/
int countMagnets() {
  float magnetCount = 0;
  int lastHallState = digitalRead(hallEffectPin);
  unsigned long timerStarted = millis();
  
  while (millis() - timerStarted < 3000) {  // 3 seconds
    int currentHallState = digitalRead(hallEffectPin);
    
    if (currentHallState != lastHallState) {
      // A change in the Hall Effect sensor state indicates a magnet was detected, participates for half the change.
      magnetCount = magnetCount + 0.5;
      Serial.print("Magnet detected. Total magnets: ");
      Serial.println(magnetCount);
      
      // Reset the timer
      timerStarted = millis();
    }
    
    lastHallState = currentHallState;
  }
  return int(floor(lastHallState));
}


/*
  Checks for 3 seconds if the Button has been pressed. If not, we go back to the loop.
*/
void Buttonpressed(int btnnumber){
  int waittimer = millis();
  if (btnnumber == 0){
    while (digitalRead(buttonPin) != HIGH){
      if ((millis() - waittimer) > 3000){
        break;
      }
    }
    if (digitalRead(buttonPin) == HIGH){
      normalvalve();
    }
  }
  else{
    //check if button was pressed
    int buttonState = digitalRead(buttonPin);
    while (buttonState != HIGH){
      if ((millis() - waittimer) > 20000){
        break;
      }
      else {
      }
    
    }
    if (buttonState == HIGH){
      //duration of opened valve, I am missing the formula I need to use
    float waterflow = calibrate();
    long duration = ((myArray[btnnumber]*0.001)/(waterflow/60))*1000; //Amount of water needed (in ml), so divide by 1000 to get liters, divided by the water flow in L/s (reason for dividing by 60, cause minutes. Then, *1000 to get this in ms.)
    SolanoidValve(duration);
    }
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
  digitalWrite(solenoidPin, HIGH);
  delay(duration*1000);
  digitalWrite(solenoidPin, LOW);
}
/*
  If we are in the normal valve (magnetCount = 0), we have the normal valve, which while the button is active, will stay open, until it is closed.
*/
void normalvalve(){
  digitalWrite(solenoidPin, HIGH);
  while (digitalRead(buttonPin) != LOW){
  }
  digitalWrite(solenoidPin, LOW);
}

/*
  function used to parse data in an array, which the arduino can then use for data.
*/
void CheckConnection(WiFiClient& client) {
  Serial.println("New client");
  String currentLine = "";
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      currentLine += c;
      if (c == '\n') {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, currentLine);
        if (error) {
          Serial.print("Error parsing JSON: ");
          Serial.println(error.c_str());
        } else {
          if (doc.containsKey("values")) {
            JsonArray values = doc["values"];
            for (JsonVariant value : values) {
              Serial.print("Received value: ");
              Serial.println(value.as<String>());
            }
            // Process the values as needed
          } else {
            Serial.println("Invalid JSON data.");
          }
        }
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println();
        client.println("Hello, World");
        break;
      }
    }
  }
  client.stop();
  Serial.println("Client disconnected");
}
