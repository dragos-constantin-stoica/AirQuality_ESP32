#ifndef UTILSSENSOR_H /* include guards */
#define UTILSENSOR_H

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <MQUnifiedsensor.h>

#define DHTPIN 25     // Digital pin connected to DHT22 sensor 
#define DHTTYPE DHT22     // DHT 22 (AM2302)

#define MQPIN 35 //Digital pin connected to MQ135 sensor
/************************Hardware Related Macros************************************/
#define         Board                   ("ESP-32") // Wemos ESP-32 or other board, whatever have ESP32 core.
#define         Pin                     (35)  //IO35 for your ESP32 WeMos Board, pinout here: https://i.pinimg.com/originals/66/9a/61/669a618d9435c702f4b67e12c40a11b8.jpg
/***********************Software Related Macros************************************/
#define         Type                    ("MQ-135") //MQ3 or other MQ Sensor, if change this verify your a and b values.
#define         Voltage_Resolution      (3.3) // 3V3 <- IMPORTANT. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define         ADC_Bit_Resolution      (12) // ESP-32 bit resolution. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define         RatioMQ135CleanAir      (3.6)//RS / R0 = 3.6 ppm  


// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview
extern DHT_Unified dht;

/*****************************Globals***********************************************/
extern MQUnifiedsensor MQ135;
/*****************************Globals***********************************************/


struct SENSOR_DATA {
  float temperature;
  float humidity;
  float CO;
  float CO2;
  float alcohol;
  float toluen;
  float NH4;
  float acetone;
  String ts;
};


/* Init sensors */
void initDHT22();
void initMQ135();

/* Read data from sensors */
void readDTH22();
void readMQ135();

#endif /* UTILSSENSOR_H */
