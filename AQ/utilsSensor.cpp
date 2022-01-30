// Include the necessary libraries
#include "utilsSensor.h"
#include "utilsTime.h"
#include "utilsSD.h"
#include <Arduino.h>


DHT_Unified dht(DHTPIN, DHTTYPE);
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
SENSOR_DATA last_reading = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };


/* Init sensors */
void initDHT22() {
  // Initialize device.
  dht.begin();
  Serial.println(F("DHT22 Unified Sensor"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  //delayMS = sensor.min_delay / 1000;
  logSDCard("[SENSOR]","initDHT22: " + String(sensor.name) + " initialization successful");  

};

void initMQ135() {

  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b

  /*****************************  MQ Init ********************************************/
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/
  MQ135.init();
  /*
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ135.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/
  // Explanation:
  // In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  // and now is on clean air (Calibration conditions), and it will setup R0 value.
  // We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  // This routine not need to execute to every restart, you can load your R0 if you know the value
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating MQ135 please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i ++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply");
    logSDCard("[SENSOR]","initMQ135: Warning - Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply");  
    while(1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply");
    logSDCard("[SENSOR]","initMQ135: Warning - Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply");  
    while(1);
  }
  /*****************************  MQ CAlibration ********************************************/
  Serial.println(F("------------------------------------"));
  logSDCard("[SENSOR]","initMQ135: MQ135 initialization successful");  

};

/* Read data from sensors */
void readDTH22() {
  // Get temperature event and print its value.
  sensors_event_t event;

  Serial.println("*** Readings from DHT-22 ****");
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    logSDCard("[SENSOR]","readDTH22: ERROR reading temperature!");  
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    last_reading.temperature = event.temperature;
    last_reading.ts = getTimeStamp();
    logSDCard("[SENSOR]","readDTH22: T: " + String(event.temperature) + "C");  
  }

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
    logSDCard("[SENSOR]","readDTH22: ERROR reading humidity!");  
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    last_reading.humidity = event.relative_humidity;
    last_reading.ts = getTimeStamp();

    logSDCard("[SENSOR]","readDTH22: H2O: " + String(event.relative_humidity) + "%");      
  }
  Serial.println("******************************");

};

float configureMQ135(float A, float B) {
  MQ135.setA(A);
  MQ135.setB(B);
  return MQ135.readSensor();
}

void readMQ135() {
  MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin

  /*
    Exponential regression:
    GAS      | a      | b
    CO       | 605.18 | -3.937
    Alcohol  | 77.255 | -3.18
    CO2      | 110.47 | -2.862
    Tolueno  | 44.947 | -3.445
    NH4      | 102.2  | -2.473
    Acetona  | 34.668 | -3.369
  */

  //MQ135.setA(605.18); MQ135.setB(-3.937); // Configurate the ecuation values to get CO concentration
  //last_reading->CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  last_reading.CO = configureMQ135(605.18, -3.937);

  // MQ135->setA(77.255); MQ135->setB(-3.18); // Configurate the ecuation values to get Alcohol concentration
  // last_reading->alcohol = MQ135->readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  last_reading.alcohol = configureMQ135(77.255, -3.937);

  //MQ135->setA(110.47); MQ135->setB(-2.862); // Configurate the ecuation values to get CO2 concentration
  //last_reading->CO2 = MQ135->readSensor() + 400.0; // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  // Note: 200 Offset for CO2 source: https://github.com/miguel5612/MQSensorsLib/issues/29
  /*
    Motivation:
    We have added 200 PPM because when the library is calibrated it assumes the current state of the
    air as 0 PPM, and it is considered today that the CO2 present in the atmosphere is around 400 PPM.
    https://www.lavanguardia.com/natural/20190514/462242832581/concentracion-dioxido-cabono-co2-atmosfera-bate-record-historia-humanidad.html
  */
  last_reading.CO2 = configureMQ135(110.47, -2.862) + 400.00;

  //MQ135->setA(44.947); MQ135->setB(-3.445); // Configurate the ecuation values to get Tolueno concentration
  //last_reading->toluen = MQ135->readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  last_reading.toluen = configureMQ135(44.947, -3.445);

  //MQ135->setA(102.2 ); MQ135->setB(-2.473); // Configurate the ecuation values to get NH4 concentration
  //last_reading->NH4 = MQ135->readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  last_reading.NH4 = configureMQ135(102.2, -2.473);

  //MQ135->setA(34.668); MQ135->setB(-3.369); // Configurate the ecuation values to get Acetona concentration
  //last_reading->acettone = MQ135->readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  last_reading.acetone = configureMQ135(34.668, -3.369);
  last_reading.ts = getTimeStamp();


  Serial.println("*** Readings from MQ-135 ****");
  Serial.print("CO: "); Serial.println(last_reading.CO);
  Serial.print("Alcohol: "); Serial.println(last_reading.alcohol);
  Serial.print("CO2 "); Serial.println(last_reading.CO2);
  Serial.print("Toluen: "); Serial.println(last_reading.toluen);
  Serial.print("NH4: "); Serial.println(last_reading.NH4);
  Serial.print("Acetone: "); Serial.println(last_reading.acetone);
  Serial.println("******************************");
  logSDCard("[SENSOR]","readMQ135: CO: " + String(last_reading.CO) + 
  " | Alcohol: " + String(last_reading.alcohol) + 
  " | CO2: " + String(last_reading.CO2) + 
  " | Toluene: " + String(last_reading.toluen) + 
  " | Amonium: " + String(last_reading.NH4) + 
  " | Acetone: " + String(last_reading.acetone));  

};
