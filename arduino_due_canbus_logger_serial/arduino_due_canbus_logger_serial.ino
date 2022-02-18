#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <SPI.h>

//#define SPEED CAN_33KBPS
#define SPEED CAN_95KBPS
//#define SPEED CAN_500KBPS

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
// Set CS to pin 9
MCP_CAN CANBUS(9);

void setup() {
	Serial.begin(38400);
	// Inizializzo can bus : baudrate = 95k, 16MHz
	if( CANBUS.begin(SPEED, MCP_16MHz) == CAN_OK ) {
		Serial.print("CAN BUS OK! 95K");
	} else {
		Serial.print("CAN BUS Error!");
	}
	// Setting pin 2 for /INT input
	pinMode(2, INPUT);
}

void loop() {
  
	if(!digitalRead(2)) {
    // Leggo il messaggio ricevuto
    // Lettura dati: len = data length, buf = data byte(s)
    CANBUS.readMsgBuf(&len, rxBuf);
    // Recupero il message ID
    rxId = CANBUS.getCanId();
    // 
    Serial.print("[");
    Serial.print(rxId);
    for(int i = 0; i<len; i++) {
      Serial.print(":");
      Serial.print(rxBuf[i]);
    }
    Serial.print("]");
    delay(20);
	}
}
