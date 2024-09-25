#include "dateFunctions.h"
#include "wifiFunctions.h"
#include "firebaseFunctions.h"
#include <FirebaseJson.h>

//Pines
const int sensor = 32;  // GPIO36
const int rele = 33;    // GPIO33

//Sensors
int soilMoistureValue = 0;
int scaleValue = 0;

//Timers
unsigned long previousReadTime = 0;
unsigned long previousSendTime = 0;

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(rele, OUTPUT);
  digitalWrite(rele, HIGH); //Starts with the relay off

  if (connectToWiFi()) {
    initTimeClient();

    FirebaseJson jsonData;

    String formattedDateTime = getFormattedDateTime();
    String route = "/connect/" + getFormattedDateYear() + "/" + getFormattedDateMonth() + "/" + getFormattedDateDay();

    jsonData.add("dateTime", getFormattedDateTime());
    jsonData.add("ip",  getIP());
    jsonData.add("message", "Success connect");

    initializeFirebase();
    sendToFirebase(route, jsonData);
  }
}

void loop() {
  unsigned long currentTime = millis(); //Gets the current time in milliseconds
  const int readInterval = 10000; //10 Seconds
  const int sendInterval = 60000; //1 Minute

  if (currentTime - previousReadTime >= readInterval) { //Sensor reading every 1 second
    previousReadTime = currentTime;
    soilMoistureValue = analogRead(sensor); //Real value
    scaleValue = map(soilMoistureValue, 0, 1023, 0, 100); //Value on a scale of 0 and 100
    //Serial.print("Soil moisture reading: ");
    //Serial.println(soilMoistureValue);
  }

  if (currentTime - previousSendTime >= sendInterval) { //Monitors and sends data every 1 minute
    previousSendTime = currentTime;

    if (connectToWiFi()) {
      initTimeClient();

      FirebaseJson jsonData;
      FirebaseJson jsonExecution;
      FirebaseJsonData jsonResult;
      String subRoute = ("/" + getFormattedDateYear() + "/" + getFormattedDateMonth() + "/" + getFormattedDateDay() + "/" + getFormattedDateHourMinute());

      //Sends the value captured by the sensor to firebase
      jsonData.add("soilMoistureValue", soilMoistureValue);
      jsonData.add("soilMoisturePercentage", scaleValue);
      sendToFirebase(("/period" + subRoute), jsonData);

      jsonData = getToFirebase("/request");

      bool execution = false;

      if (jsonData.get(jsonResult, "execution")) {

        if (jsonResult.boolValue) {
          execution = jsonResult.boolValue;
        }

        if (execution) { //Activates the irrigation pump and updates the database
          ActivatePumping(7);
          jsonData.set("execution", false);
          sendToFirebase("/request", jsonData);

          jsonExecution.add("action", "manual");
          jsonExecution.add("ip",  getIP());
          jsonExecution.add("dateTime", getFormattedDateTime());
          sendToFirebase(("/irrigate" + subRoute), jsonExecution);

        } else if (scaleValue <= 70) { //Activate irrigation if humidity is low
          ActivatePumping(10);

          jsonExecution.add("action", "automatic");
          jsonExecution.add("ip",  getIP());
          jsonExecution.add("dateTime", getFormattedDateTime());
          sendToFirebase(("/irrigate" + subRoute), jsonExecution);
        }
      } else {
        Serial.println("Error getting 'execution' value!");
      }
    }
  }
}

void ActivatePumping(const int &timeoutSeconds) {
  int timeExecution = timeoutSeconds * 1000;

  Serial.println("Pump connected by " + String(timeoutSeconds) + " seconds...");

  digitalWrite(rele, LOW); //Turn on the relay
  delay(timeExecution);
  digitalWrite(rele, HIGH);

  Serial.println("Pump off!");
}