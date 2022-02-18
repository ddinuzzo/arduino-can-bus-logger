#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SD.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
// Set CS to pin 9
MCP_CAN CAN0(9);
// Definizione parametri display
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Definizione file per il salvataggio dei dati
File logFile;

void setup() {
	lcd.begin();
	lcd.backlight();

	lcd.print("DND-READER v0.3d");
	lcd.setCursor(0, 1);
	Serial.begin(115200);
	if (SD.begin(4) == true) {
		logFile = SD.open("pids.dnd", FILE_WRITE);
		logFile.println("*****************************************");
		logFile.println("******** DND CANBUS READER v0.3d ********");
		logFile.println("*****************************************");
		logFile.close();
		lcd.print("SD Card OK!");
	} else {
		lcd.print("SD Card error!!");
	}
	// Ritardo di 2 secondi per poter vedere se l'inizializzazione della scheda SD è avvenuta con successo
	delay(2000);
	lcd.setCursor(0, 1);
	// Inizializzo can bus : baudrate = 500k, 16MHz
	if( CAN0.begin(CAN_500KBPS,MCP_16MHz) == CAN_OK ) {
		lcd.print("CAN BUS OK!");
	} else {
		lcd.print("CAN BUS Error!");
	}
	// Setting pin 2 for /INT input
	pinMode(2, INPUT);
}
// Mode da inserire nella richiesta
unsigned char mod_pid[2] = { 
	0x01, // Realtime data
	0x03  // Visualizza i codici diagnostica salvati
};
// Pids da da inserire nella richiesta
unsigned char pids[2][6] = {
	{
		0x05, // Temperatura liquido refrigerante del motore
		0x0C, // Numero di giri per minuto del motore
		//0x04, // Carico del motore
		0x0D, // Velocità dell'auto
		0x01, // Restituisce l'esito dei check in fase di inizializzazione della centralina (contiene anche informazioni sullo stato del MIL)
		//0x03, // Restituisce informazioni sullo stato del sistema di alimentazione primario e secondario (se presente)
		//0x06, // Short term fuel trim—Bank 1
		//0x07, // Long term fuel trim—Bank 1
		//0x0E, // Tempi di anticipo
		//0x0F, // Temperatura aria aspirata
		//0x10, // Portata d'aria MAF
		//0x11, // Posizione corpo farfallato
		//0x13, // Oxygen sensors present (in 2 banks)
		//0x14, // Sensore Ossigeno 1, byte A: Voltaggio, byte B: Short term fuel trim
		//0x15, // Sensore Ossigeno 2, byte A: Voltaggio, byte B: Short term fuel trim
		//0x1C, // Standard OBD supportato dal veicolo
		//0x21, // Distanza percorsa con il MIL (spia check motore) accesa
		//0x2E, // Commanded evaporative purge
		0x2F, // Livello benzina nel serbatoio
		//0x3C, // Catalyst Temperature: Bank 1, Sensor 1
		//0x42, // Monitor status this drive cycle
		//0x43, // Absolute load value
		//0x44, // Fuel–Air commanded equivalence ratio
		//0x45, // Relative throttle position
		//0x49, // Accelerator pedal position D
		//0x4A, // Accelerator pedal position E
		//0x4C, // Commanded throttle actuator
		'\n'
	}, {
		0x00, // In questa modalità non è richiesto alcun pid
		'\n'  // Informazione di uscita
	}
};
// Pacchetto che viene inviato alla ECU (viene modificato mode e pid)
//                       id,   mode, pid
unsigned char stmp[8] = {0x02, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned int engine_rpm = 0;
unsigned int veicle_speed = 0;
int end = 0;

void loop() {
  logFile = SD.open("pids.dnd", FILE_WRITE);
	// Ciclo sulle madalità
	for(int modeIdx = 0 ; modeIdx < sizeof(mod_pid)/sizeof(unsigned char) ; modeIdx++) {
		// Imposto la modalità
		stmp[1] = mod_pid[modeIdx];
		// Inizio a ciclare sui pid inseriti all'interno dell'array pids
		for(int pidIdx = 0 ; pidIdx < sizeof(pids[modeIdx])/sizeof(unsigned char) ; pidIdx++) {
			if(pids[modeIdx][pidIdx] != '\n'  ) {
				end = 0;
				// Recupero il pid in posizione modeIdx x pidIdx e lo inserisco nella request da inviare alla ECU
				stmp[2] = pids[modeIdx][pidIdx];   
				// Invio la request per il pid
				RESEND_DATA:
				CAN0.sendMsgBuf(0x7DF, 0x04, 8, stmp);
				delay(100);
				// Se il pin 2 è in stato "LOW" leggo i buffer ricevuto
				READ_DATA:
				if(!digitalRead(2)) {
					// Leggo il messaggio ricevuto
					// Lettura dati: len = data length, buf = data byte(s)
					CAN0.readMsgBuf(&len, rxBuf);
					// Valuto il pacchetto solo se proviene dalla ECU (0x7E8) ed
					// ha lo stesso pid della richiesta.
					if( rxBuf[2] == stmp[2] ) {
						// Recupero il message ID
						rxId = CAN0.getCanId();
						// effettuo il salvataggio dei dati nel file
						logFile.print("ID: ");
						logFile.print(rxId, HEX);
						logFile.print(" --> DATA: ");
						for(int i = 0; i<len; i++) {    
							if(rxBuf[i] < 0x10) {
								logFile.print("0");
							}
							logFile.print(rxBuf[i], HEX);
							logFile.print(" ");
						}
						logFile.println();
						logFile.println("-----------------------------------------");

						if(rxBuf[1] == 0x41 && stmp[2] == 0x0C) { // Informazione numero di giri del motore
							engine_rpm = (rxBuf[3] * 256);
							engine_rpm = engine_rpm + rxBuf[4];
							engine_rpm = engine_rpm/4;
							lcd.setCursor(0, 0);
							lcd.print("Giri: ");
							lcd.print(engine_rpm);
							lcd.print(" rpm      ");
						} else if (rxBuf[1] == 0x41 && stmp[2] == 0x0D) { // informazione velocità auto
							veicle_speed = rxBuf[3];
							lcd.setCursor(0, 1);
							lcd.print("Vel: ");
							lcd.print(veicle_speed);
							lcd.print(" Km/h      ");
						}
					} else {
						if(end++ < 10) {
						  goto READ_DATA;
						}
					}
				} else {
					// se non ho ottenuto alcun pacchetto reinvio la richiesta e mi metto in ascolto
					if(end++ < 5) {
						goto RESEND_DATA;
					}
				}
			} else {
				break;
			}
		}
	}
 logFile.close();
}
