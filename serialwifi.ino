/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*  EEPROM implementation test
/*****************************************************************************/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "webpage.h"
#include "FIFO.h"

static const int  KB	= 1024;
static const int  PORT  = 80;
static const int  SERIAL_BAUDRATE = 115200;
static const int  EEPROM_SIZE	  = 330;			//Size can be anywhere between 4 and 4096 bytes
static const int  ROM_BANK_SIZE	  = 30;			//bytes long
static const int  SERIAL_TIMEOUT  = 1000;		//ms
static const char*  wifiSSID	  = "(-_-)";
static const char*  wifiPassword  = "monteiro";
static const char*  hostName	  = "serialwifi";

String emailAddress;
String faultCommand;
String baudRateOption;
String dataFormatRadio;

String serialBuffer;
MemoryBuffer dataBuffer;

ADC_MODE(ADC_VCC);
ESP8266WebServer server(PORT);


void handleRoot() {
	Serial.println("Handling request: Index..");
	
	// get static webpage part residing in flash memory ~4Kb
	String Index = FPSTR(HTTP_WEBSITE);	

	Serial.println("replacing text strings..");

	Index.replace("{{dataBuffer}}", htmlReplaceSpecialChars(dataBuffer.ReadStringFromBuffer()));
	Index.replace("{{wifiSSID}}", htmlReplaceSpecialChars(String(wifiSSID)));
	Index.replace("{{ipAddress}}", ipAddress());
	Index.replace("{{powerSupply}}", powerSupply());
	Index.replace("{{emailAddress}}", htmlReplaceSpecialChars(emailAddress));
	Index.replace("{{faultCommand}}", htmlReplaceSpecialChars(faultCommand));
	Index.replace("{{freeHeap}}", freeHeap());
	Index.replace("{{" + baudRateOption + "}}", "selected");
	Index.replace("{{" + dataFormatRadio + "}}", "checked");
	Index.replace("{{usedRam}}", String((float)(Index.length() + dataBuffer.GetCurrentSize()) / KB));

	server.sendHeader("Content-Length", String(Index.length()));
	server.send(200, "text/html", Index);
	Serial.println("Request sent!");
}

void handleSave() {

	if (server.arg(4).toInt()) {				//button			
		Serial.println("Saving server data..");	
		emailAddress = server.arg(0);			//text1
		faultCommand = server.arg(1);			//text2
		baudRateOption = server.arg(2);			//option
		dataFormatRadio = server.arg(3);		//radio

		EEPROM_SAVE(1, emailAddress);			//save values to EEPROM;
		EEPROM_SAVE(2, faultCommand);
		EEPROM_SAVE(3, baudRateOption);
		EEPROM_SAVE(4, dataFormatRadio);

		EEPROM_SAVE(10, "ROM_OK");
	}

	Serial.println("Done! Restarting..");
	server.sendHeader("Location", "/", true); //redirect to prevent resubmission
	server.send(302, "text/plain", "");

	ESP.reset();
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

String ipAddress() {
	return WiFi.localIP().toString();
}

String debugInfo() {
	return String(millis()) + ":" + freeHeap() + ":" + powerSupply() + ":: ";
}

String powerSupply() {
	return String((float)ESP.getVcc() / KB);
}

String freeHeap() {
	return String((float)ESP.getFreeHeap() / KB);
}

String htmlReplaceSpecialChars(String NewData) {
	NewData.replace("&", "&amp;");
	NewData.replace(">", "&gt;");
	NewData.replace("<", "&lt;");
	NewData.replace("<", "&lt;");
	NewData.replace("\'", "&apos;");
	NewData.replace("\"", "&quot;");
	return NewData;
}

void setSerialBaudrate(String option) {

	option.remove(0,6); //option0
	int baud = option.toInt(); 

	//Serial.print("Changing serial baud to: ");
	//Serial.println(baud);

	switch (baud)
	{

	case 0: baud = 115200; break;
	case 1: baud = 57600; break;
	case 2: baud = 38400; break;
	case 3: baud = 28800; break;
	case 4: baud = 19200; break;
	case 5: baud = 14400; break;
	case 6: baud = 9600; break;
	case 7: baud = 4800; break;
	case 8: baud = 2400; break;
	case 9: baud = 1200; break;

	default:
		baud = 115200; break;
	}

	Serial.flush();
	Serial.begin(baud, SERIAL_8N1);
}

void setup(void) {
	Serial.setTimeout(SERIAL_TIMEOUT);
	//Serial.begin(SERIAL_BAUDRATE, SERIAL_8N1);
	setSerialBaudrate("");
	Serial.println();
	Serial.println("=== ESP-01 Restart ===");
	Serial.print("Serial Init. Baudrate: ");
	Serial.println(SERIAL_BAUDRATE);

	EEPROM.begin(EEPROM_SIZE);
	Serial.print("Eeprom Conf. Size: ");
	Serial.println(EEPROM_SIZE);
	

	if (romIsEmpty()){
		Serial.println("Rom Empty.");
		emailAddress = "";
		faultCommand = "";
		baudRateOption = "";
		dataFormatRadio = "";
	}
	else{
		Serial.println("Rom not Empty.");
		emailAddress = EEPROM_READ(1);
		faultCommand = EEPROM_READ(2);
		baudRateOption = EEPROM_READ(3);
		dataFormatRadio = EEPROM_READ(4);
	}

	setSerialBaudrate(baudRateOption);
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


	Serial.print("IP address: ");
	Serial.println(ipAddress());

	server.on("/", HTTP_GET, handleRoot);  // when client requests webpage
	server.on("/", HTTP_POST, handleSave); // when client saves changes
	server.onNotFound(handleNotFound);     // When client requests an unknown webpage

	server.begin();
	Serial.println("HTTP server started.");
}

void loop(void) {

	//Serial.println(debugInfo());

	server.handleClient();


	while (Serial.available()) {

			serialBuffer += char(Serial.read()); //gets one byte from serial buffer
			
			if (serialBuffer.endsWith("\n")) { // check string termination
					if (serialBuffer.indexOf(faultCommand) >= 0) { //lookup for command
						serialBuffer += " --> FAULT FOUND! <-- \n";
					}
				dataBuffer.WriteStringToBuffer(serialBuffer); //write to buffer

				serialBuffer = "";
			}
			
		}
		
		//dataBuffer.WriteStringToBuffer(Serial.readString());
		//dataBuffer.WriteByteToBuffer(Serial.read());

	delay(100);
}
