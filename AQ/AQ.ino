//
//
//  Air Quality (AQ)
//
//  This is a prototype for air quality monitoring system.
//  It runs on an ESP32 platform with
//  - MQ135 sensor for gas
//  - DHT22 sensor for temperature and humidity
//
//  @author: Dragoş STOICA
//  @email: dragos.constantin.stoica@outlook.com
//  @date: 07.09.2020
//

/****************************************
   Include Libraries
 ****************************************/
#include "utilsSD.h"
#include "utilsCfg.h"
#include "utilsTime.h"
#include "utilsOLED.h"
#include "utilsSensor.h"
#include "utilsWiFi.h"


extern bool IS_WIFI;
//-------------------------------------------------------------------------
// Prepare the device:
// - sync time
// - initialize sensors
// - start local webserver
//-------------------------------------------------------------------------
void setup() {
  //init and get the time
  setenv("TZ", "  CET-1CEST,M3.5.0,M10.5.0/3", 1);
  // You must include '0' after first designator e.g. GMT0GMT-1, ',1' is true or ON

  Serial.begin(115200);
  Serial.println("'Oo._");
  Serial.println("*** Program Start : ESP32 with MQ135 and DHT22");
  /*
     The main init sequence

     0. Init SD card
        - read WiFi config from aq.cfg
        - read IoT config form aq.cfg
        - write log file airquality.log
     1. Init OLED
     2. Init Sensors
     3. Init WiFi
        - init Webserver if WiFi config present
        - init IoT send data if IoT config present

     Deal with RTC init
      > initialize RTC first
      > consider that RTC is OK and use it from the beggining
      > when WiFi connection is established do synchronize the RTC with web time server
  */
  init_RTC();
  sd_init();
  cfg_init();
  oled_init();

  // Initialize temperature and air quality sensors
  initDHT22();
  oled_addline("DHT22 [OK]");
  initMQ135();
  oled_addline("MQ135 [OK]");

  if (IS_WIFI) {
    oled_addline("AQ monitoring [OK]");
    wifi_init();
  } else {
    oled_addline("AQ monitoring !NOK!");
    oled_addline(">... SYSTEM HALT!");
    oled_display();
  }
}

void loop() {
  if (IS_WIFI) {
    wifi_run();
  }
}
