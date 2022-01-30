#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#include "utilsOLED.h"
#include "utilsSD.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int crtrow = 0;
String lcdlines[8];

void oled_init() {

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    logSDCard("[OLED]","init: ERROR SSD1306 allocation failed");
    for (;;)
      ;
  }

  Serial.println(F("[OLED init] SSD1306 allocation successful"));
  logSDCard("[OLED]","init: SSD1306 allocation successful");
  
  display.display();
  delay(500);
  display.clearDisplay();
}

void oled_display() {
  //8 lines of text at TextSize 1
  //use a rotating buffer with 8 slots
  //push up new lines

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println(lcdlines[0]);
  display.println(lcdlines[1]);
  display.println(lcdlines[2]);
  display.println(lcdlines[3]);
  display.println(lcdlines[4]);
  display.println(lcdlines[5]);
  display.println(lcdlines[6]);
  display.println(lcdlines[7]);

  display.display();
  //delay(2000);
}

void oled_addline(String line) {
  if (crtrow > 7) {
    //push all rows up
    lcdlines[0] = "";
    lcdlines[1] = "";
    lcdlines[2] = "";
    lcdlines[3] = "";
    lcdlines[4] = "";
    lcdlines[5] = "";
    lcdlines[6] = "";
    lcdlines[7] = "";
    crtrow=0;
    lcdlines[crtrow] = line;
  } else {
    //fill up the lcd lines
    lcdlines[crtrow] = line;
    crtrow++;
  }

  
}
