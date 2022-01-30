// Include the necessary libraries
#include <HTTPClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>
#include "ESPAsyncWebServer.h"
#include "time.h"

#include "utilsOLED.h"
#include "utilsSD.h"
#include "utilsSensor.h"
#include "utilsTime.h"
#include "utilsWiFi.h"

// Task scheduling configurations
#define WIFI_CONNECT_INTERVAL   5000     //Connection retry interval in milliseconds
#define WIFI_WATCHDOG_INTERVAL  600000   //Wi-Fi watchdog interval in milliseconds
#define WIFI_SENDDATA_INTERVAL  1200000  //Wi-Fi send data inverval in milliseconds
#define OLED_DISPLAY_INTERVAL   5000     //OLED refresh interval in milliseconds


//GLOBAL VARIABLES

//Your Domain name with URL path or IP address with path
extern char* serverName;

//This will work only if not behind a corporate firewall
extern char* ntpServer;
extern int   gmtOffset_sec;
extern int   daylightOffset_sec;

extern char* WIFI_SSID;
extern char* WIFI_PASS;

extern SENSOR_DATA last_reading;

//uBeac key values for HTTP authenticaiton
extern char* APIKEY;
extern char* APIKEYVALUE;
extern char* uBeacAPI;
extern char* deviceID;

extern bool IS_uBEAC;


// Tasks
Task wifi_connect_task(WIFI_CONNECT_INTERVAL, TASK_FOREVER, &wifi_connect_cb);
Task wifi_watchdog_task(WIFI_WATCHDOG_INTERVAL, TASK_FOREVER, &wifi_watchdog_cb);
Task wifi_senddata_task(WIFI_SENDDATA_INTERVAL, TASK_FOREVER, &send_data_cb);
Task oled_display_task(OLED_DISPLAY_INTERVAL, TASK_FOREVER, &oled_display);

// Task runner
Scheduler runner;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with DHT values
String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATURE") {
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    // Read temperature as Celsius (the default)
    float t = last_reading.temperature;
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float t = dht.readTemperature(true);
    // Check if any reads failed and exit early (to try again).
    if (isnan(t)) {
      Serial.println("Failed to read from DHT22 sensor!");
      return "--";
    }
    else {
      //Serial.println(t);
      oled_addline("T:" + String(t) + "C");
      return String(t);
    }
  }
  else if (var == "HUMIDITY") {
    float h = last_reading.humidity;
    if (isnan(h)) {
      Serial.println("Failed to read from DHT22 sensor!");
      return "--";
    }
    else {
      //Serial.println(h);
      oled_addline("H2O:" + String(h) + "%");
      return String(h);
    }
  }
  else if (var == "AIR") {
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float aq = last_reading.CO2;
    if (isnan(aq)) {
      Serial.println("Failed to read from MQ135 sensor!");
      return "--";
    }
    else {
      //Serial.println(aq);
      oled_addline("CO2:" + String(aq) + " ppm");
      return String(aq);
    }
  } else if (var == "TIMESTAMP") {
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    return (last_reading.ts);
  }
  return String();
}


// Wi-Fi events
void _wifi_event(WiFiEvent_t event) {
  switch (event)
  {
    case SYSTEM_EVENT_WIFI_READY:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi interface ready\n"));
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      // Serial.print(PSTR("[Wi-Fi] Event: Completed scan for access points\n"));
      break;
    case SYSTEM_EVENT_STA_START:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi client started\n"));
      break;
    case SYSTEM_EVENT_STA_STOP:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi clients stopped\n"));
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      // Serial.printf(PSTR("[Wi-Fi] Event: Connected to access point: %s \n"), WiFi.localIP().toString().c_str());
      logSDCard("[WiFi]", "_wifi_event: Connected to access point: " + WiFi.localIP().toString());
      wifi_connect_task.disable();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      // Serial.print(PSTR("[Wi-Fi] Event: Not connected to Wi-Fi network\n"));
      wifi_connect_task.enable();
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      // Serial.print(PSTR("[Wi-Fi] Event: Authentication mode of access point has changed\n"));
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      // Serial.printf(PSTR("[Wi-Fi] Event: Obtained IP address: %s\n"), WiFi.localIP().toString().c_str());
      logSDCard("[WiFi]", "_wifi_event: Obtained IP address: " + WiFi.localIP().toString());
      wifi_watchdog_task.enable();
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      // Serial.print(PSTR("[Wi-Fi] Event: Lost IP address and IP address is reset to 0\n"));
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi Protected Setup (WPS): succeeded in enrollee mode\n"));
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi Protected Setup (WPS): failed in enrollee mode\n"));
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi Protected Setup (WPS): timeout in enrollee mode\n"));
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi Protected Setup (WPS): pin code in enrollee mode\n"));
      break;
    case SYSTEM_EVENT_AP_START:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi access point started\n"));
      break;
    case SYSTEM_EVENT_AP_STOP:
      // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi access point  stopped\n"));
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      // Serial.print(PSTR("[Wi-Fi] Event: Client connected\n"));
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      // Serial.print(PSTR("[Wi-Fi] Event: Client disconnected\n"));
      break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
      // Serial.print(PSTR("[Wi-Fi] Event: Assigned IP address to client\n"));
      break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
      // Serial.print(PSTR("[Wi-Fi] Event: Received probe request\n"));
      break;
    case SYSTEM_EVENT_GOT_IP6:
      // Serial.print(PSTR("[Wi-Fi] Event: IPv6 is preferred\n"));
      break;
    case SYSTEM_EVENT_ETH_START:
      // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_START\n"));
      break;
    case SYSTEM_EVENT_ETH_STOP:
      // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_STOP\n"));
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_CONNECTED\n"));
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_DISCONNECTED\n"));
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_GOT_IP\n"));
      break;
    case SYSTEM_EVENT_MAX:
      logSDCard("[WiFi]", "_wifi_event: SYSTEM_EVENT_MAX");
      break;
  }
}

// Wi-Fi connect task
void wifi_connect_cb() {
  // Disable this task to avoid further iterations
  wifi_connect_task.disable();

  Serial.println(PSTR("[Wi-Fi] Status: Connecting ..."));

  // Disconnect from Wi-Fi network
  WiFi.disconnect();

  // Connect to Wi-Fi network
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for connection result and capture the result code
  uint8_t result = WiFi.waitForConnectResult();

  // Serial.printf("[Wi-Fi] Connection Result: %d \n", result);

  if (result == WL_NO_SSID_AVAIL || result == WL_CONNECT_FAILED)
  {
    // Fail to connect or SSID no available
    Serial.println(PSTR("[Wi-Fi] Status: Could not connect to Wi-Fi AP"));
    logSDCard("[WiFi]", "wifi_connect_cb: Warning - Could not connect to Wi-Fi AP");
    // Wait and reenable this task to keep trying to connect
    wifi_connect_task.enableDelayed(WIFI_CONNECT_INTERVAL);
  }
  else if (result == WL_IDLE_STATUS)
  {
    // Wi-Fi Idle. This means that it's connected to the AP but the DHCP has not assigned an IP yet
    Serial.println(PSTR("[Wi-Fi] Status: Idle | No IP assigned by DHCP Server"));
    logSDCard("[WiFi]", "wifi_connect_cb: Warning - Idle | No IP assigned by DHCP Server. Disconnecting.");
    WiFi.disconnect(); // Optional to disconnect and start again instead of wait for DHCP to assign IP.
  }
  else if (result == WL_CONNECTED)
  {
    // Wi-Fi Connected
    Serial.printf(PSTR("[Wi-Fi] Status: Connected | IP: %s\n"), WiFi.localIP().toString().c_str());
    logSDCard("[WiFi]", "wifi_connect_cb: Connected! IP: " + WiFi.localIP().toString());
    oled_addline(String("IP: ") + WiFi.localIP().toString().c_str());
    init_NTP();
    readDTH22();
    readMQ135();
    // Start server
    server.begin();
  }
}

// Wi-Fi watchdog task
void wifi_watchdog_cb() {
  Serial.println(PSTR("[Watchdog] Checking Wi-Fi ..."));
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(PSTR("[Watchdog] Wi-Fi Disconnected"));
    logSDCard("[WiFi]", "wifi_watchdog_cb: Warning - Wi-Fi Disconnected.");
    // Disconnect Wi-Fi
    WiFi.disconnect();
    // Disable watchdog task
    wifi_watchdog_task.disable();
    // Enable Wi-Fi connect task to reconnect to Wi-Fi AP
    wifi_connect_task.enableDelayed(WIFI_CONNECT_INTERVAL);
    return;
  }
  Serial.println(PSTR("[Watchdog] Wi-Fi is connected!"));
  logSDCard("[WiFi]", "wifi_watchdog_cb: Wi-Fi is connected!");
}

// Wi-Fi post data to Internet
void send_data_cb() {
  // Block until we are able to connect to the WiFi access point
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Posting JSON data to server...");
    logSDCard("[WiFi]", "send_data_cb: Posting JSON data to server: " + String(uBeacAPI));

    readDTH22();
    readMQ135();

    HTTPClient http;

    http.begin(uBeacAPI);
    http.addHeader("Content-Type", "application/json");
    http.addHeader(APIKEY, APIKEYVALUE);

    StaticJsonDocument<500> doc;
    // allocate the memory for the document
    StaticJsonDocument<100> temperature;
    StaticJsonDocument<100> humidity;
    StaticJsonDocument<100> airquality;

    // Add values in the document root
    //
    doc["ip"] = deviceID;

    // Add an array of senosrs
    //
    JsonArray data = doc.createNestedArray("sensors");

    // create an object
    temperature["uid"] = "temperature";
    temperature["unit"] = 2;
    temperature["prefix"] = 0;
    temperature["Type"] = 4;
    temperature["value"] = last_reading.temperature;

    humidity["uid"] = "humidity";
    humidity["unit"] = 20;
    humidity["prefix"] = 0;
    humidity["Type"] = 5;
    humidity["value"] = last_reading.humidity;

    airquality["uid"] = "airquality";
    airquality["unit"] = 6;
    airquality["prefix"] = 0;
    airquality["Type"] = 13;
    airquality["value"] = last_reading.CO2;

    data.add(temperature);
    data.add(humidity);
    data.add(airquality);

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
      Serial.println(httpResponseCode);
      http.end();
    }
    else {
      Serial.printf("Error occurred while sending HTTP POST: %s\n", http.errorToString(httpResponseCode).c_str());
      logSDCard("[WiFi]", "send_data_cb: ERROR while sending HTTP POST: " + http.errorToString(httpResponseCode));
    }

    //RTC time check
    if (!RTC_Valid()) {
      RTC_Update();
    };

    //printDateTime();
  }
}

void wifi_init() {

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SD, "/index.html", "text/html");
  });
  server.serveStatic("/", SD, "/");

  server.on("/api", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String key;
    if (request->hasParam("key")) {
      key = request->getParam("key")->value();
    } else {
      key = "EMPTY";
    }
    request->send(200, "text/plain", processor(key));
  });

  logSDCard("[WiFi]", "init: HTTP server running");

  // Prepare task runner
  runner.init();
  runner.addTask(wifi_connect_task);
  runner.addTask(wifi_watchdog_task);
  if (IS_uBEAC) {
    runner.addTask(wifi_senddata_task);
  }
  runner.addTask(oled_display_task);

  // Set Wi-Fi mode
  WiFi.mode(wifi_mode_t::WIFI_STA);

  // Wi-Fi events listener
  WiFi.onEvent(_wifi_event);

  // Wait and enable wifi connect task
  wifi_connect_task.enableDelayed(WIFI_CONNECT_INTERVAL);
  wifi_watchdog_task.enableDelayed(WIFI_WATCHDOG_INTERVAL);
  // Wait and enable wifi send data task
  if (IS_uBEAC) {
    wifi_senddata_task.enableDelayed(WIFI_SENDDATA_INTERVAL);
  }
  //Wait and enable OLED display task
  oled_display_task.enableDelayed(OLED_DISPLAY_INTERVAL);
  logSDCard("[WiFi]", "init: Tasks scheduled setup");

  Serial.printf(PSTR("[Info] Connection Interval: %d(ms) \n"), WIFI_CONNECT_INTERVAL);
  Serial.printf(PSTR("[Info] Wi-Fi Watchdog Interval: %d(ms) \n"), WIFI_WATCHDOG_INTERVAL);
  Serial.printf(PSTR("[Info] Wi-Fi Send Data Interval: %d(ms) \n"), WIFI_SENDDATA_INTERVAL);
  Serial.printf(PSTR("[Info] OLED Display Interval: %d(ms) \n"), OLED_DISPLAY_INTERVAL);
}

void wifi_run() {
  runner.execute();
}
