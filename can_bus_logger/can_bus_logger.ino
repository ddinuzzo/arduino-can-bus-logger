#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SD.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[30];
// Set CS to pin 9
MCP_CAN CAN0(9);
// Definizione parametri display
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Definizione file per il salvataggio dei dati
File logFile;

void setup() {
	lcd.begin();
	lcd.backlight();

	lcd.print("DND-READER v0.3e");
	lcd.setCursor(0, 1);
	Serial.begin(38400);
	if (SD.begin(4) == true) {
		logFile = SD.open("pids.dnd", FILE_WRITE);
		logFile.println("*****************************************");
		logFile.println("******** DND CANBUS READER v0.3e ********");
		logFile.println("*****************************************");
		logFile.close();
		lcd.print("SD Card OK!");
	} else {
		lcd.print("SD Card Error!!");
	}
	// Ritardo di 2 secondi per poter vedere se l'inizializzazione della scheda SD � avvenuta con successo
	delay(2000);
	lcd.setCursor(0, 1);
	// Inizializzo can bus : baudrate = 95k, 16MHz
  START_INIT:
	if( CAN0.begin(CAN_95kBPS,MCP_16MHz) == CAN_OK ) {
		lcd.print("CAN BUS OK! 95K");
    Serial.println("CAN BUS Shield init ok!>");
	} else {
		lcd.print("CAN BUS Error!");
    Serial.println("CAN BUS Shield init fail>");
    Serial.println("Init CAN BUS Shield again>");
    delay(100);
    goto START_INIT;
	}
	// Setting pin 2 for /INT input
	pinMode(2, INPUT);
}

unsigned char stmp[8] = {0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void loop() {
  CAN0.sendMsgBuf(0x7DF, 0x04, 8, stmp);
  delay(100);
  
	if(!digitalRead(2)) {
    lcd.clear();
    logFile = SD.open("pids.dnd", FILE_WRITE);
    // Leggo il messaggio ricevuto
    // Lettura dati: len = data length, buf = data byte(s)
    CAN0.readMsgBuf(&len, rxBuf);
    // Recupero il message ID
    rxId = CAN0.getCanId();
    // effettuo il salvataggio dei dati nel file
    // e contestualmente lo presento sul display
    logFile.print("ID: ");
    lcd.print("ID: ");
    logFile.print(rxId, HEX);
    lcd.print(rxId, HEX);
    logFile.print(" --> DATA: ");
    lcd.setCursor(0, 1);
    lcd.print("DATA: ");
    Serial.print("<");Serial.print(rxId);Serial.print(",");
    for(int i = 0; i<len; i++) {    
      if(rxBuf[i] < 0x10) {
        logFile.print("0");
        lcd.print("0");
      }
      logFile.print(rxBuf[i], HEX);
      lcd.print(rxBuf[i], HEX);
      logFile.print(" ");
      lcd.print(" ");
      Serial.print(rxBuf[i]);Serial.print(",");
    }
    Serial.print(">");
    Serial.println();
    logFile.println();
    logFile.println("-----------------------------------------");
    logFile.close();
	}
}
