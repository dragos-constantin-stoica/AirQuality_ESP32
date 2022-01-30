#ifndef UTILSSD_H /* include guards */
#define UTILSD_H

// SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>

void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void logSDCard(String module, String message);
void sd_init();


#endif /* UTILSD_H */
