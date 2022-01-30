#include <HTTPClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>


// RTC Libraries
#include <Wire.h>
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

#include "utilsTime.h"
#include "utilsSD.h"

//GLOBAL VARIABLES

//Your Domain name with URL path or IP address with path
extern char *serverName;

//This will work only if not behind a corporate firewall
extern char *ntpServer;
extern int   gmtOffset_sec;
extern int   daylightOffset_sec;



//RTC section

void printDateTime() {
  char datestring[24];
  RtcDateTime dt = Rtc.GetDateTime();
  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u UTC"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.println(datestring);
}

String getTimeStamp(){
  char datestring[24];
  RtcDateTime dt = Rtc.GetDateTime();
  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u UTC"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  return String(datestring);
}

void RTC_Update() {
  // Do udp NTP lookup, epoch time is unix time - subtract the 30 extra yrs (946684800UL) library expects 2000
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[Update RTC] Getting time from Internet ...");
    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(serverName);

    // Send HTTP GET request
    int httpResponseCode = http.GET();
    String payload = "{}";

    if (httpResponseCode > 0) {
      Serial.print("[Update RTC] HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.print("[Update RTC] HTTP Error code: ");
      Serial.println(httpResponseCode);
    }

    StaticJsonDocument<1000> timeJSON;                         //Memory pool
    DeserializationError error = deserializeJson(timeJSON, payload);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("[Update RTC]deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    Serial.println(timeJSON["unixtime"].as<long>());

    http.end();

    unsigned long epoch = timeJSON["unixtime"].as<long>();
    unsigned long epochTime = epoch - 946684800UL;
    Rtc.SetDateTime(epochTime);

    printDateTime();
  }

}

bool RTC_Valid() {
  bool boolCheck = true;
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!  Updating DateTime");
    boolCheck = false;
    RTC_Update();
  }
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now.  Updating Date Time");
    Rtc.SetIsRunning(true);
    boolCheck = false;
    RTC_Update();
  }
  return boolCheck;
}

//END RTC section

/**
  Print time from ESP32 internal clock
*/
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}


void init_RTC() {
    //Initialize RTC clock
    Rtc.Begin();
    Serial.println("[RTC] init: RTC initialized");
};


/**
   Initialize ESP32 internal clock from internet
   NOTE: NTP does dot work behind corporate firewall
*/
void init_NTP(){

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[Init TIME] Getting time from Internet ...");
    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(serverName);

    // Send HTTP GET request
    int httpResponseCode = http.GET();
    String payload = "{}";

    if (httpResponseCode > 0) {
      Serial.print("[Init TIME] HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
      Serial.print("[Init TIME] HTTP Response body: ");
      Serial.println(payload);
    } else {
      Serial.print("[Init TIME] HTTP Response ERROR code: ");
      Serial.println(httpResponseCode);
      return;
    }

    StaticJsonDocument<1000> timeJSON;                         //Memory pool
    DeserializationError error = deserializeJson(timeJSON, payload);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("[Init TIME]deserializeJson() failed: "));
      Serial.println(error.c_str());
      logSDCard("[RTC]","initNTP: ERROR deserializeJson() failed: " + String(error.c_str()));
      return;
    }

    //For debug pourpouse
    //Serial.println(timeJSON["unixtime"].as<long>());

    http.end();

    //Does not work behind firewall :(
    //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    timeval epoch = {timeJSON["unixtime"].as<long>(), 0};
    settimeofday((const timeval*)&epoch, 0);
    printLocalTime();

    //Set RTC clock
    if (!Rtc.GetIsRunning()) {
      Serial.println(F("[RTC Clock] Initialization failed"));
      logSDCard("[RTC]","initNTP: ERROR RTC initialization failed");
    };

    Serial.print(F("[RTC Clock] Before update from Internet: "));
    printDateTime();

    unsigned long epochTime = timeJSON["unixtime"].as<unsigned long>() - 946684800UL;
    Rtc.SetDateTime(epochTime);
    printDateTime();
    logSDCard("[RTC]","initNTP: RTC and ESP32 internal clock initialized successfully from NTP server " + String(serverName));

  }
  
}
