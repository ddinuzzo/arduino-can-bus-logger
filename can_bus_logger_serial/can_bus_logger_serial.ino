#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <SPI.h>
#include <Wire.h>

//#define SPEED CAN_33KBPS
//Low Speed CAN 33.3kbps
//LSCAN-H - 1
//LSCAN-L - GND - 5
/***********************/
//#define SPEED CAN_95KBPS
//Medium Speed CAN 95kbps
//MSCAN-H - 3
//MSCAN-L - 11
/***********************/
//#define SPEED CAN_500KBPS
//High Speed CAN 500kbps
//HSCAN-H - 6
//HSCAN-L - 14

#define resetPin 8

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
// Set CS to pin 9
MCP_CAN CANBUS(9);

void setup() {
	Serial.begin(38400);
  // Settaggio pin per reset interfaccia
  digitalWrite(resetPin, HIGH);
  pinMode(resetPin, OUTPUT); 
  // Settaggio velocita canbus
  int SPEED;
  CONFIGURE_BITRATE:
  if (Serial.available() > 0) {
    char canbus_speed = (char)Serial.read();
    if( canbus_speed == '1' ) {
      SPEED = CAN_5KBPS;
    } else if( canbus_speed == '2' ) {
      SPEED = CAN_10KBPS;
    } else if( canbus_speed == '3' ) {
      SPEED = CAN_20KBPS;
    } else if( canbus_speed == '4' ) {
      SPEED = CAN_31K25BPS;
    } else if( canbus_speed == '5' ) {
      SPEED = CAN_33KBPS;
    } else if( canbus_speed == '6' ) {
      SPEED = CAN_40KBPS;
    } else if( canbus_speed == '7' ) {
      SPEED = CAN_50KBPS;
    } else if( canbus_speed == '8' ) {
      SPEED = CAN_80KBPS;
    } else if( canbus_speed == '9' ) {
      SPEED = CAN_83K3BPS;  
    } else if( canbus_speed == 'a' ) {
      SPEED = CAN_95KBPS;
    } else if( canbus_speed == 'b' ) {
      SPEED = CAN_100KBPS;
    } else if( canbus_speed == 'c' ) {
      SPEED = CAN_125KBPS;
    } else if( canbus_speed == 'd' ) {
      SPEED = CAN_200KBPS;
    } else if( canbus_speed == 'e' ) {
      SPEED = CAN_250KBPS;
    } else if( canbus_speed == 'f' ) {
      SPEED = CAN_500KBPS;
    } else if( canbus_speed == 'g' ) {
      SPEED = CAN_666KBPS;
    } else if( canbus_speed == 'h' ) {
      SPEED = CAN_1000KBPS;
    } else {
      goto CONFIGURE_BITRATE;
    }
  } else {
    goto CONFIGURE_BITRATE;
  }
  
  START_INIT:
	if( CANBUS.begin(SPEED,MCP_16MHz) == CAN_OK ) {		
    //Serial.println("CAN BUS Shield init ok!>");
	} else {
    //Serial.println("CAN BUS Shield init fail>");
    //Serial.println("Init CAN BUS Shield again>");
    delay(100);
    goto START_INIT;
	}
	// Setting pin 2 for /INT input
	pinMode(2, INPUT);

  Serial.setTimeout(50);
}

void loop() {
	if(!digitalRead(2)) {
    // Leggo il messaggio ricevuto
    // Lettura dati: len = data length, buf = data byte(s)
    CANBUS.readMsgBuf(&len, rxBuf);
    // Recupero il message ID
    rxId = CANBUS.getCanId();
    Serial.print("[");Serial.print(rxId);
    for(int i = 0; i<len; i++) {
      Serial.print(":");Serial.print(rxBuf[i]);
    }
    Serial.print("]");
	}

  if (Serial.available() > 0) {
    int pkId = Serial.parseInt(), dIdx = 0;
    if( pkId == 0 ) {
      // Se ricevo un pkId == 0 resetto l'interfaccia
      // mi serve per poter gestire un cambio di configurazione
      // dal software (cambio velocità canbus)
      digitalWrite(resetPin, LOW);
    }
    byte stdPkg = (byte) Serial.parseInt(), aLen = (byte) Serial.parseInt();
    byte packet[aLen];
    while(int val = Serial.parseInt()) {
      packet[dIdx++] = (byte) val;
    }
    /** INVIA PACCHETTO **/
    CANBUS.sendMsgBuf(pkId, stdPkg, aLen, packet);
  }
}
