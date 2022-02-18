#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include "mcp_can.h"

//  \ 1  2  3  4  5  6  7  8  /
//   \9 10 11 12 13 14 15 16/

//#define SPEED CAN_33KBPS
//Low Speed CAN 33.3kbps
//LSCAN-H - 1
//LSCAN-L - GND - 5

//#define SPEED CAN_95KBPS
//Medium Speed CAN 95kbps
//MSCAN-H - 3
//MSCAN-L - 11

#define SPEED CAN_500KBPS
//High Speed CAN 500kbps
//HSCAN-H - 6
//HSCAN-L - 14

MCP_CAN CAN(9);

INT32U canId = 0x000;

unsigned char len = 0;
unsigned char buf[8];
char str[20];

LiquidCrystal_I2C lcd(0x27, 16, 2);
File logFile;

void setup() {
    Serial.begin(38400);
    lcd.begin();
    lcd.backlight();
    if (SD.begin(4) == true) {
      lcd.print("SD Initialized!");
    } else {
      lcd.print("SD Error!");
    }
START_INIT:
    lcd.setCursor(0, 1);
    //MidSpeed BUS - 95.23kpbs PIN 6 - CAN-H, PIN 14 CAN - L
    if(CAN_OK == CAN.begin(SPEED)) {
        lcd.print("CAN BUS OK");
    } else {
        lcd.print("CAN BUS Error");
        delay(100);
        goto START_INIT;
    }
}

void loop() {
    if(CAN_MSGAVAIL == CAN.checkReceive()) {
        CAN.readMsgBuf(&len, buf); 
        canId = CAN.getCanId();

        lcd.clear();
        lcd.print("ID: ");
        lcd.print(canId);
        lcd.setCursor(0, 1);
        lcd.print("DATA: ");
        for(int i = 0; i<len; i++) {
            if(buf[i] < 0x10) {
              lcd.print("0");
            }
            lcd.print(buf[i]);
            lcd.print(" ");
        };
        if (SD.begin(4) == true) {
          logFile = SD.open("pids.dnd", FILE_WRITE);
          if(logFile) {
            logFile.print("ID: ");
            logFile.print(canId, HEX);
            logFile.print(" --> DATA: ");
            for(int i = 0; i<len; i++) {
                if(buf[i] < 0x10) {
                  logFile.print("0");
                }
                logFile.print(buf[i], HEX);
                logFile.print(" ");
            }
            logFile.println();
            logFile.println("-----------------------------------------");
            logFile.close();
          }
        }
    }
}
