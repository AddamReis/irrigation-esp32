#include "dateFunctions.h"
#include "wifiFunctions.h"
#include "firebaseFunctions.h"
#include <FirebaseJson.h>

//Pines
const int sensor1 = 32;  // GPIO36
const int sensor2 = 35;  // GPIO35
const int relay1 = 33;   // GPIO33
const int relay2 = 25;   // GPIO25

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
  const int readInterval = 60000;          //1 Minute
  const int sendIntervalMinutes = 600000;  //10 Minutes
  const int sendIntervalHour = 3600000;    //1 Hour

  if (currentTime - previousReadTime >= readInterval) {  //sensors reading every 1 Minute
    initTimeClient();
    previousReadTime = currentTime;

    soilMoistureValue1 = analogRead(sensor1);  //Real value
    soilMoistureValue2 = analogRead(sensor2);
    soilMoistureValueMean = ((soilMoistureValue1 + soilMoistureValue2) / 2);

    scaleValue1 = map(soilMoistureValue1, 0, 1023, 0, 100);  //Value on a scale of 0 and 100
    scaleValue2 = map(soilMoistureValue2, 0, 1023, 0, 100);
    scaleValueMean = ((scaleValue1 + scaleValue2) / 2);

    Serial.println("Soil Moisture Value 1: " + String(soilMoistureValue1) + " | Mean: " + String(scaleValue1));
    Serial.println("Soil Moisture Value 2: " + String(soilMoistureValue2) + " | Mean: " + String(scaleValue2));
    Serial.println("Soil Moisture Mean: " + String(soilMoistureValueMean) + " | Mean: " + String(scaleValueMean));

    if (currentTime - previousSendTimeMinutes >= sendIntervalMinutes) {  //sensors reading every 10 Minutes
      previousSendTimeMinutes = currentTime;
      FirebaseJson jsonData;

      if (connectToWiFi()) {
        initTimeClient();
        subRoute = ("/" + getFormattedDateYear() + "/" + getFormattedDateMonth() + "/" + getFormattedDateDay() + "/" + getFormattedDateHourMinute());

        jsonData.add("soilMoistureValue1", soilMoistureValue1);  //Sends the value captured by the sensor to firebase
        jsonData.add("soilMoisturePercentage1", scaleValue1);
        jsonData.add("soilMoistureValue2", soilMoistureValue2);
        jsonData.add("soilMoisturePercentage2", scaleValue2);
        jsonData.add("soilMoistureValueMean", soilMoistureValueMean);
        jsonData.add("soilMoisturePercentageMean", scaleValueMean);
        sendToFirebase(("/period" + subRoute), jsonData);

        if (currentTime - previousSendTimeHour >= sendIntervalHour) {  //Monitors and sends data every 1 hour
          previousSendTimeHour = currentTime;
          FirebaseJsonData jsonResult;
          FirebaseJson jsonExecution;

          jsonData = getToFirebase("/request");
          execution = false;

          if (jsonData.get(jsonResult, "execution")) {
            execution = jsonResult.boolValue;

            if (execution) {  //Activates the irrigation pump and updates the database
              ActivatePumping(10, relay1);
              ActivatePumping(10, relay2);

              jsonData.set("execution", false);
              sendToFirebase("/request", jsonData);

              jsonExecution.add("action", "manual");
              jsonExecution.add("ip", getIP());
              jsonExecution.add("dateTime", getFormattedDateTime());
              sendToFirebase(("/irrigate" + subRoute), jsonExecution);
            } else {
              if (scaleValue1 <= 70)
                ActivatePumping(10, relay1);

              if (scaleValue2 <= 70)
                ActivatePumping(10, relay2);

              if (execution) {
                jsonExecution.add("action", "automatic");
                jsonExecution.add("ip", getIP());
                jsonExecution.add("dateTime", getFormattedDateTime());
                sendToFirebase(("/irrigate" + subRoute), jsonExecution);
              }
            }
          } else {
            Serial.println("Error getting 'execution' value!");
          }
        }
      }
    }
  }
}

void ActivatePumping(const int &timeoutSeconds, const int relay) {
  execution = true;
  Serial.println("Pump connected by " + String(timeoutSeconds) + " seconds...");

  digitalWrite(relay, LOW);  //Turn on the relay
  delay(timeoutSeconds * 1000);
  digitalWrite(relay, HIGH);

  Serial.println("Pump off!");
}