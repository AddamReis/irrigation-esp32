#ifndef DATEFUNCTIONS_H
#define DATEFUNCTIONS_H

#include <Arduino.h>

void initTimeClient();
String getFormattedDateTime();
String getFormattedDateYear();
String getFormattedDateMonth();
String getFormattedDateDay();
String getFormattedDateHourMinute();

#endif