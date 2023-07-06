#include <SoftwareSerial.h>
#include <math.h>
#include <RTClib.h>
#include <SPI.h>

RTC_PCF8523 rtc;

#include <Adafruit_Thermal.h>

Adafruit_Thermal printer(@@@@@)

#include <SD.h>

enum ErrorType {
    SDError,
    createFileError
}