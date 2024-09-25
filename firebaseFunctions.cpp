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

  Serial.print("Sending data to Firebase... ");
  bool status = Database.set(client, path, object_t(jsonString.c_str()));

  if (status) {
    Serial.println("Data sent successfully!");
    delay(2000);
    return true;
  } else {
    Serial.println("Error: " + String(client.lastError().code()) + ", Message: " + client.lastError().message());
    return false;
  }
}

FirebaseJson getToFirebase(const String &path) {
  FirebaseJson jsonData;
  String jsonString = Database.get<String>(client, path);

  if (!jsonString.isEmpty()) {
    jsonData.setJsonData(jsonString);
    Serial.println("Data received: " + jsonString);
    return jsonData;
  } else {
    Serial.println("Error retrieving data from Firebase!");
    return jsonData;
  }
}