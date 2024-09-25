#include "dateFunctions.h"
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const long utcOffsetInSeconds = -3 * 3600; //BR time zone

WiFiUDP ntpUDP; //Initialize the UDP client and NTP client
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void initTimeClient() {
    timeClient.begin();
}

String getFormattedDateTime() {
  timeClient.update();
  int currentYear = year(timeClient.getEpochTime());
  int currentMonth = month(timeClient.getEpochTime());
  int currentDay = day(timeClient.getEpochTime());
  int currentHour = hour(timeClient.getEpochTime());
  int currentMinute = minute(timeClient.getEpochTime());

  char dateTime[20];
  sprintf(dateTime, "%04d%02d%02d-%02d:%02d", currentYear, currentMonth, currentDay, currentHour, currentMinute);

  return String(dateTime);
}

String getFormattedDateYear() {
  timeClient.update();
  int currentYear = year(timeClient.getEpochTime());
  char dateTime[20];
  sprintf(dateTime, "%04d", currentYear);
  return String(dateTime);
}

String getFormattedDateMonth() {
  timeClient.update();
  int currentMonth = month(timeClient.getEpochTime());
  char dateTime[20];
  sprintf(dateTime, "%02d", currentMonth);
  return String(dateTime);
}

String getFormattedDateDay() {
  timeClient.update();
  int currentDay = day(timeClient.getEpochTime());
  char dateTime[20];
  sprintf(dateTime, "%02d", currentDay);
  return String(dateTime);
}

String getFormattedDateHourMinute() {
  timeClient.update();
  int currentHour = hour(timeClient.getEpochTime());
  int currentMinute = minute(timeClient.getEpochTime());
  char dateTime[20];
  sprintf(dateTime, "%02d-%02d", currentHour, currentMinute);
  return String(dateTime);
}