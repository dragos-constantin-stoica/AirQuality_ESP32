#include "utilsSD.h"
#include "utilsTime.h"


#define log_file_name "/airquality.log"

/**
   SD Card utils
*/

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    //Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

// Write the sensor readings on the SD card
void logSDCard(String module, String message) {
  String dataMessage ="[" + String(getTimeStamp()) + "]: - " +  String(module) + "::" + String(message) + "\r\n";
  Serial.printf(PSTR("Save data: %s\n"),dataMessage.c_str());
  appendFile(SD, log_file_name, dataMessage.c_str());
}


void sd_init() {
  if (!SD.begin()) {
    Serial.println("[SD Card Init] SD Card Mount Failed");
    return;
  }

  writeFile(SD, log_file_name, ("[" + String(getTimeStamp())+ "]: - "+ "[SD]::init: Air Quality application started\r\n").c_str());
}

//END SD Card
