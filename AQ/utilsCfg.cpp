#include <SDConfig.h>
#include "utilsCfg.h"
#include "utilsOLED.h"

SDConfig cfg;

//GLOBAL VARIABLES

//Your Domain name with URL path or IP address with path
char* serverName;

//This will work only if not behind a corporate firewall
char* ntpServer;
int   gmtOffset_sec;
int   daylightOffset_sec;

char* WIFI_SSID;
char* WIFI_PASS;

char* APIKEY;
char* APIKEYVALUE;
char* uBeacAPI;
char* deviceID;

bool IS_WIFI=false;
bool IS_uBEAC=false;
//END of GLOBAL variables

void cfg_init() {
  if (cfg.begin(CFG_FILE_NAME, 128)) {
    while (cfg.readNextSetting()) {
      if (cfg.nameIs("serverName")) {
        serverName = cfg.copyValue();
        Serial.printf(PSTR("[CFG] serverName: %s \n"), serverName);
      } else if (cfg.nameIs("ntpServer")) {
        ntpServer = cfg.copyValue();
        Serial.printf(PSTR("[CFG] ntpServer: %s \n"), ntpServer);
      } else if (cfg.nameIs("gmtOffset_sec")) {
        gmtOffset_sec = cfg.getIntValue();
        Serial.printf(PSTR("[CFG] gmtOffset: %d \n"), gmtOffset_sec);
      } else if (cfg.nameIs("daylightOffset_sec")) {
        daylightOffset_sec = cfg.getIntValue();
        Serial.printf(PSTR("[CFG] daylightOffset: %d \n"), daylightOffset_sec);
      } else if (cfg.nameIs("WIFI_SSID")) {
        WIFI_SSID = cfg.copyValue();
        Serial.printf(PSTR("[CFG] WIFI SSID: %s \n"), WIFI_SSID);
      } else if (cfg.nameIs("WIFI_PASS")) {
        WIFI_PASS = cfg.copyValue();
        Serial.printf(PSTR("[CFG] WIFI PASS: %s \n"), WIFI_PASS);        
      } else if (cfg.nameIs("APIKEY")) {
        APIKEY = cfg.copyValue();
        Serial.printf(PSTR("[CFG] uBeac API key: %s \n"), APIKEY);        
      } else if (cfg.nameIs("APIKEYVALUE")) {
        APIKEYVALUE = cfg.copyValue();
        Serial.printf(PSTR("[CFG] uBeac API key value: %s \n"), APIKEYVALUE);        
      } else if (cfg.nameIs("uBeacAPI")) {
        uBeacAPI = cfg.copyValue();
        Serial.printf(PSTR("[CFG] uBeac API endpoint: %s \n"), uBeacAPI);        
      } else if (cfg.nameIs("deviceID")) {
        deviceID = cfg.copyValue();
        Serial.printf(PSTR("[CFG] uBeac device ID: %s \n"), deviceID);        
      } else {
        Serial.printf(PSTR("[CFG]:Not processed, the name of this setting is: %s \n"), cfg.getName());
      }
    }
    //Check configuration and decide what modules to initialize
    if(WIFI_SSID != NULL){
        oled_addline("WiFi SSID [OK]");
        IS_WIFI=true;      
    }else{
      oled_addline("WiFi !NOK! Check CFG");
    }
  if ((APIKEY != NULL) && (APIKEYVALUE != NULL) && (uBeacAPI != NULL) && (deviceID != NULL)){
    oled_addline("uBeac setup [OK]");    
    IS_uBEAC=true;
  }else{
    oled_addline("uBeac not configured");
  }
    cfg.end();
  }else{
    Serial.printf(PSTR("[CFG]: Failed to open configuration file: %s\n"), CFG_FILE_NAME);
  }
}
