/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*  EEPROM implementation test
/*****************************************************************************/

#define KB		 1024
#define PORT	 80
#define SERIAL_BAUDRATE 115200
#define SERIAL_BUFFER 512
//#define MAX_LINES 100		//40*100 = 4KB
#define BUFFER_SIZE 7000 //Bytes
#define EEPROM_SIZE 330		//Size can be anywhere between 4 and 4096 bytes
#define ROM_BANK_SIZE 30	//chars = bytes
#define SERIAL_TIMEOUT 1000 //ms

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "webpage.h"
#include "FIFO.h"

const char* wifiSSID	 = "(-_-)";
const char* wifiPassword = "monteiro";
const char* hostName	 = "serialwifi";

String emailAddress;
String faultCommand;

String serialBuffer;
MemoryBuffer dataBuffer;

String ipAddress;
String freeHeap;
String powerSupply;
String debugInfo;

ADC_MODE(ADC_VCC);
ESP8266WebServer server(PORT);


void handleRoot() {
	Serial.println("Handling request: Index..");
	
	String Index = FPSTR(HTTP_WEBSITE);	// get static webpage part residing in flash memory
	Serial.println("replacing strings..");

	Index.replace("{{wifiSSID}}", String(wifiSSID));
	Index.replace("{{ipAddress}}", ipAddress);
	Index.replace("{{freeHeap}}", freeHeap);
	Index.replace("{{powerSupply}}", powerSupply);
	Index.replace("{{emailAddress}}", emailAddress);
	Index.replace("{{faultCommand}}", faultCommand);
	Index.replace("{{dataBuffer}}", dataBuffer.ReadStringFromBuffer());

	server.sendHeader("Content-Length", String(Index.length()));
	server.send(200, "text/html", Index);
	Serial.println("Request sent!");

}

void handleSave() {

	Serial.print("Saving server data..");	//save values to EEPROM;
	emailAddress = server.arg("text1");
	faultCommand = server.arg("text2");
	EEPROM_SAVE(1, emailAddress);
	EEPROM_SAVE(2, faultCommand);
	EEPROM_SAVE(10, "ROM_OK");
	Serial.println("Done!");

	server.sendHeader("Location", "/", true); //redirect to prevent resubmission
	server.send(302, "text/plain", "");


	//ESP.restart();
}

void handleNotFound() {
	Serial.println("Handling request: Not found");
	server.send(404, "text/plain", "404: Not found");
}

void EEPROM_SAVE(int BankNumber, String NewData) {

	BankNumber *= ROM_BANK_SIZE;
	char Data[ROM_BANK_SIZE];
	NewData.toCharArray(Data, ROM_BANK_SIZE);

	for (int i = 0; i < ROM_BANK_SIZE; i++) {
		EEPROM.write(BankNumber + i, Data[i]);
		//Serial.print(char(Data[i]));

		if (Data[i] == 0) break;
	}

	EEPROM.commit();
}

String EEPROM_READ(int BankNumber) {

	BankNumber *= ROM_BANK_SIZE;
	char Data[ROM_BANK_SIZE];

	for (int i = 0; i < ROM_BANK_SIZE; i++) {
		Data[i] = EEPROM.read(BankNumber + i);
		//Serial.print(char(Data[i]));

		if (Data[i] == 0) break;
	}

	return String(Data);
}

boolean romIsEmpty() {
	if (EEPROM_READ(10) == "ROM_OK") {
		return false;
		}
	else {
		return true;
		}
}

void setup(void) {
	Serial.setTimeout(SERIAL_TIMEOUT);
	Serial.begin(115200, SERIAL_8N1);
	Serial.println();
	Serial.println("=== uP Restart ===");
	Serial.print("Serial Init. Baudrate: ");
	Serial.println(SERIAL_BAUDRATE);

	EEPROM.begin(EEPROM_SIZE);
	Serial.print("Eeprom Conf. Size: ");
	Serial.println(EEPROM_SIZE);

	

	if (romIsEmpty()){
		Serial.println("Rom Empty.");
		emailAddress = "";
		faultCommand = "";
	}
	else{
		Serial.println("Rom not Empty.");
		emailAddress = EEPROM_READ(1);
		faultCommand = EEPROM_READ(2);
	}

	Serial.println("Network Init..");
	WiFi.hostname(hostName);
	Serial.print("Netbios: ");
	Serial.println(hostName);

	WiFi.begin(wifiSSID, wifiPassword);
	Serial.print("Connecting to: ");
	Serial.println(wifiSSID);

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();
	Serial.println("Connected!");
	ipAddress = WiFi.localIP().toString();

	Serial.print("IP address: ");
	Serial.println(ipAddress);

	server.on("/", HTTP_GET, handleRoot);  // when client requests webpage
	server.on("/", HTTP_POST, handleSave); // when client saves changes
	server.onNotFound(handleNotFound);     // When client requests an unknown webpage

	server.begin();
	Serial.println("HTTP server started.");
}

void loop(void) {

	server.handleClient();

	freeHeap = String((float)ESP.getFreeHeap() / KB);
	powerSupply = String((float)ESP.getVcc() / KB);
  debugInfo = String(millis()) + ":" + freeHeap + ":" + powerSupply + ":: ";


	while (Serial.available()) {

			serialBuffer += char(Serial.read()); //gets one byte from serial buffer
			
			if (serialBuffer.endsWith("\n")) { // check string termination
				dataBuffer.WriteStringToBuffer(serialBuffer); //write to buffer
					if (serialBuffer.indexOf(faultCommand) >= 0) { //lookup for command
						Serial.println("FAULT FOUND!");
					}
				serialBuffer = "";
			}
			
		}
		
		//dataBuffer.WriteStringToBuffer(Serial.readString());
		//dataBuffer.WriteByteToBuffer(Serial.read());

	delay(10);
}
