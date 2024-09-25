#ifndef FIREBASEFUNCTIONS_H
#define FIREBASEFUNCTIONS_H

#include <Arduino.h>
#include <FirebaseJson.h>

//Firebase
#define DATABASE_SECRET ""
#define FIREBASE_HOST ""

void initializeFirebase();
bool sendToFirebase(const String &path, FirebaseJson &jsonData);
FirebaseJson getToFirebase(const String &path);

#endif
