#include "firebaseFunctions.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <FBJS_Config.h>

//Firebase variables
WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));
FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
LegacyToken dbSecret(DATABASE_SECRET);

//Variables
const int maxAttempts = 3;
const int delayTime = 5000;

void initializeFirebase() {
  ssl.setInsecure();                              // Sets SSL to insecure
  initializeApp(client, app, getAuth(dbSecret));  // Launch FirebaseApp
  app.getApp<RealtimeDatabase>(Database);         // Binds the RealtimeDatabase
  Database.url(FIREBASE_HOST);                    // Set the Firebase URL
  client.setAsyncResult(result);                  // Sets the result of the async operation
}

bool sendToFirebase(const String &path, FirebaseJson &jsonData) {
  String jsonString;
  jsonData.toString(jsonString, false);

  Serial.println("Route: " + path);
  Serial.println("Data: " + jsonString);

  int attempt = 0;
  bool status = false;

  while (attempt < maxAttempts) {
    Serial.print("Attempt " + String(attempt + 1) + ": Sending data to Firebase... ");
    status = Database.set(client, path, object_t(jsonString.c_str()));

    if (status) {
      Serial.println("Data sent successfully!");
      delay(2000);
      return true;
    } else {
      Serial.println("Error: " + String(client.lastError().code()) + ", Message: " + client.lastError().message());
      attempt++;

      if (attempt < maxAttempts) {
        Serial.println("Retrying in " + String(delayTime/1000) + " seconds...");
        delay(delayTime);
      }
    }
  }

  Serial.println("Failed to send data after " + String(maxAttempts) + " attempts.");
  return false;
}

FirebaseJson getToFirebase(const String &path) {
  FirebaseJson jsonData;
  String jsonString = Database.get<String>(client, path);

  int attempt = 0;
  bool success = false;

  while (attempt < maxAttempts && !success) {
    if (!jsonString.isEmpty()) {
      jsonData.setJsonData(jsonString);
      Serial.println("Data received: " + jsonString);
      success = true;
      return jsonData;
    } else {
      Serial.println("Error retrieving data from Firebase!");
      attempt++;

      if (attempt < maxAttempts) {
        Serial.println("Retrying in " + String(delayTime/1000) + " seconds...");
        delay(delayTime);
      }
    }
  }

  Serial.println("Failed to retrieve data after " + String(maxAttempts) + " attempts.");
  return jsonData;
}