#include <mcp_can_dfs.h>

#define dataLen 9
#define packetNumber 14

#define resetPin 8

int packets[packetNumber][dataLen] = {
  { 1667, 46, 1, 119 }, 
  { 1262, 46, 3, 1, 19 },
  { 1104, 46, 7, 6, 78 },
  { 384, 46, 3, 16, 11, 76, 197, 11, 255 }, 
  { 1256, 46, 15, 21, 104, 14, 128, 1 }, 
  { 1729, 16, 58, 192, 0, 55, 3, 16, 26 }, 
  { 1729, 33, 0, 27, 0, 91, 0, 102, 0 },
  { 1729, 34, 83, 0, 95, 0, 100, 0, 109 },
  { 1729, 35, 0, 70, 0, 77, 0, 49, 0 },
  { 1729, 36, 27, 0, 91, 0, 102, 0, 83 },
  { 1729, 37, 0, 95, 0, 103, 0, 109, 0 }, //25
  { 1729, 38, 32, 0, 32, 0, 68, 0, 69 }, //26
  { 1729, 39, 0, 69, 0, 74, 0, 65, 0 }, //27
  { 1729, 40, 89, 0, 32, 0, 0, 0, 0 } //28
};

void setup() {
  digitalWrite(resetPin, HIGH);
  pinMode(resetPin, OUTPUT);  
  Serial.begin(38400);

  int canbus_speed;
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

  Serial.setTimeout(50);

  Serial.print("Selezionata velocita: ");
  Serial.println(SPEED);
}

int pkId = -1;
int aLen = -1;


void loop() {
  for(int i=0;i<packetNumber;i++) {
    Serial.print("[");
    Serial.print(packets[i][0]);
    for(int j=1;j<dataLen;j++) {
      Serial.print(":");
      Serial.print(packets[i][j]);  
    }
    Serial.print("]");
  }
  
  if (Serial.available() > 0) {
    int pkId = Serial.parseInt(), dIdx = 0;
    if( pkId == 0 ) {
      digitalWrite(resetPin, LOW);
    }
    byte stdPkg = (byte) Serial.parseInt(), aLen = (byte) Serial.parseInt();
    byte packet[aLen];
    while(int val = Serial.parseInt()) {
      packet[dIdx++] = (byte) val;
    }
    /** INVIARE PACCHETTO **/
    Serial.print("[");
    Serial.print(pkId);
    Serial.print(":");
    for(int i=0;i<aLen;i++){
      Serial.print(packet[i]);
      Serial.print(":");
    }
    Serial.println("]");
  }
}

