#include "dateFunctions.h"
#include "wifiFunctions.h"
#include "firebaseFunctions.h"
#include <FirebaseJson.h>

//Pines
const int sensor1 = 32;  // GPIO36
const int sensor2 = 35;  // GPIO35
const int relay1 = 33;   // GPIO33
const int relay2 = 25;   // GPIO25
const int relayAll[] = { 33, 25 };

//sensors
int soilMoistureValue1 = 0;
int scaleValue1 = 0;
int soilMoistureValue2 = 0;
int scaleValue2 = 0;
int soilMoistureValueMean = 0;
int scaleValueMean = 0;

//Timers
unsigned long previousReadTime = 0;
unsigned long previousSendTimeMinutes = 0;
unsigned long previousSendTimeHour = 0;

//Variables
String subRoute;
bool execution = false;

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, HIGH);  //Starts with the relay off
  digitalWrite(relay2, HIGH);

  if (connectToWiFi()) {
    initTimeClient();

    FirebaseJson jsonData;
    String route = "/connect/" + getFormattedDateYear() + "/" + getFormattedDateMonth() + "/" + getFormattedDateDay();

    jsonData.add("dateTime", getFormattedDateTime());
    jsonData.add("ip", getIP());
    jsonData.add("message", "Success connect");

    initializeFirebase();
    sendToFirebase(route, jsonData);
  }
}

void loop() {
  unsigned long currentTime = millis();    //Gets the current time in milliseconds
  const int readInterval = 120000;         //2 Minute
  const int sendIntervalMinutes = 600000;  //10 Minutes
  const int sendIntervalHour = 3600000;    //1 Hour

  if (currentTime - previousReadTime >= readInterval) {  //sensors reading every 2 Minutes
    previousReadTime = currentTime;

    if (connectToWiFi()) {
      initTimeClient();
      subRoute = ("/" + getFormattedDateYear() + "/" + getFormattedDateMonth() + "/" + getFormattedDateDay() + "/" + getFormattedDateHourMinute());

      soilMoistureValue1 = analogRead(sensor1);  //Real value
      soilMoistureValue2 = analogRead(sensor2);
      soilMoistureValueMean = ((soilMoistureValue1 + soilMoistureValue2) / 2);

      scaleValue1 = map(soilMoistureValue1, 0, 1023, 0, 100);  //Value on a scale of 0 and 100
      scaleValue2 = map(soilMoistureValue2, 0, 1023, 0, 100);
      scaleValueMean = ((scaleValue1 + scaleValue2) / 2);

      Serial.println("Soil Moisture Value 1: " + String(soilMoistureValue1) + " | Mean: " + String(scaleValue1));
      Serial.println("Soil Moisture Value 2: " + String(soilMoistureValue2) + " | Mean: " + String(scaleValue2));
      Serial.println("Soil Moisture Mean: " + String(soilMoistureValueMean) + " | Mean: " + String(scaleValueMean));

      FirebaseJsonData jsonResult;
      FirebaseJson jsonData;
      execution = false;

      handlePumpExecution("execute-all-pumps", 0, "all", subRoute, "manual", execution);

      if (execution == false) {
        handlePumpExecution("execute-pump-1", relay1, "1", subRoute, "manual", execution);
        handlePumpExecution("execute-pump-2", relay2, "2", subRoute, "manual", execution);
      }

      if (currentTime - previousSendTimeMinutes >= sendIntervalMinutes) {  //sensors reading every 10 Minutes
        previousSendTimeMinutes = currentTime;
        FirebaseJson jsonData;

        jsonData.add("soilMoistureValue1", soilMoistureValue1);  //Sends the value captured by the sensor to firebase
        jsonData.add("soilMoisturePercentage1", scaleValue1);
        jsonData.add("soilMoistureValue2", soilMoistureValue2);
        jsonData.add("soilMoisturePercentage2", scaleValue2);
        jsonData.add("soilMoistureValueMean", soilMoistureValueMean);
        jsonData.add("soilMoisturePercentageMean", scaleValueMean);
        sendToFirebase(("/period" + subRoute), jsonData);

        if (currentTime - previousSendTimeHour >= sendIntervalHour && execution == false) {  //Monitors and sends data every 1 hour
          previousSendTimeHour = currentTime;

          if (scaleValue1 <= 70)
            handlePumpExecution("execute-pump-1", relay1, "1", subRoute, "automatic", execution);

          if (scaleValue2 <= 70)
            handlePumpExecution("execute-pump-2", relay2, "2", subRoute, "automatic", execution);
        }
      }
    }
  }
}

void handlePumpExecution(const String &pumpField, int relay, const String &pumpLabel, const String &subRoute, const String &action, bool &execution) {
  FirebaseJsonData jsonResult;
  FirebaseJson jsonData;

  if (action == "manual") {
    jsonData = getToFirebase("/request");

    if (jsonData.get(jsonResult, pumpField)) {
      execution = jsonResult.boolValue;

      if (execution) {
        if (pumpField == "execute-all-pumps") {
          int arraySize = sizeof(relayAll) / sizeof(relayAll[0]);

          for (int i = 0; i < arraySize; i++) {
            ActivatePumping(10, relayAll[i]);
          }
        } else {
          ActivatePumping(10, relay);
        }

        jsonData.set(pumpField, false);
        sendToFirebase("/request", jsonData);

        jsonData.clear();
        jsonData.add("action", action);
        jsonData.add("pumps", pumpLabel);
        jsonData.add("ip", getIP());
        jsonData.add("dateTime", getFormattedDateTime());
        sendToFirebase(("/irrigate" + subRoute), jsonData);
      }
    }
  } else if (action == "automatic") {
    ActivatePumping(10, relay);

    jsonData.add("action", action);
    jsonData.add("pumps", pumpLabel);
    jsonData.add("ip", getIP());
    jsonData.add("dateTime", getFormattedDateTime());
    sendToFirebase(("/irrigate" + subRoute), jsonData);
  }
}

void ActivatePumping(const int &timeoutSeconds, const int relay) {
  Serial.println("Pump connected by " + String(timeoutSeconds) + " seconds...");

  digitalWrite(relay, LOW);  //Turn on the relay
  delay(timeoutSeconds * 1000);
  digitalWrite(relay, HIGH);

  Serial.println("Pump off!");
}