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
#define interruptPin 2
#define interfacePin 9

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
// Set CS to pin 9
MCP_CAN CANBUS(interfacePin);

const char CMD[] = "AI+";
const char CMD_SPEED[] = "SPEED";
const char CMD_NAME[] = "NAME";
const char CMD_VERSION[] = "VERSION";
const char CMD_STATUS[] = "STATUS";
const char CMD_SEND[] = "SEND";

const char VERSION[] = "v0";
const char NAME[] = "ArduInterface";

String command;
int SPEED = -1;
bool isActive = false;
bool isReady = false;

void setup() {
  Serial.begin(38400);

  // Settaggio pin per reset interfaccia
  digitalWrite(resetPin, HIGH);
  pinMode(resetPin, OUTPUT);
  // Settaggio pin per interrupt
  pinMode(interruptPin, INPUT);
}

void loop() {
  // Verifico se sono presenti valori sulla seriale
  if (Serial.available() > 0) {
    // Recupero il comando
    while(Serial.available() > 0) {
      command += Serial.read();
    }
    parseCommand( command );
    command = "";
  }

  if( isReady && isActive ) {
    if(!digitalRead(interruptPin)) {
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
  }
}

void parseCommand( String command ) {
  bool isSetCommand = (command.indexOf("=") > -1);
  // Verifico se si tratta di uno dei comandi per settare dei dati
  if( command.indexOf( CMD_SPEED ) > -1 ) {
    if( isSetCommand ) {
      String sValue = getCommandValue( command );
      if( sValue != NULL ) {
        char val[5];
        sValue.toCharArray(val, 5);
        SPEED = atoi(val);
        if( CANBUS.begin(SPEED, MCP_16MHz) == CAN_OK ) {    
          isReady = true;
        } else {
          isReady = false;
        }
      }
    } else {
      printCommand(CMD_SPEED, SPEED);
    }
  } else if( command.indexOf( CMD_NAME ) > -1 ) {
    printCommand(CMD_NAME, NAME);
  } else if( command.indexOf( CMD_VERSION ) > -1 ) {
    printCommand(CMD_VERSION, VERSION);
  } else if( command.indexOf( CMD_STATUS ) > -1 ) {
    if( isSetCommand && isReady ) {
      String sValue = getCommandValue( command );
      if( sValue == NULL ) {
        isActive = (sValue == "1");
      } else {
        isActive = false;
      }
    } else {
      String interfaceStatus = isReady ? (isActive ? "1" : "0") : "-1";
      printCommand(CMD_STATUS, interfaceStatus);
    }
  } else if( command.indexOf( CMD_SEND ) > -1 ) {
    String sValue = getCommandValue( command );
  }
}

String getCommandValue(String command) {
  return command.substring( command.indexOf( "=" ) + 1 );
}

void printCommand(String commandName, String commanValue) {
  Serial.print(CMD);
  Serial.print(commandName);
  Serial.print("=");
  Serial.println(commanValue);
}

void printCommand(String commandName, int commanValue) {
  Serial.print(CMD);
  Serial.print(commandName);
  Serial.print("=");
  Serial.println(commanValue);
}
