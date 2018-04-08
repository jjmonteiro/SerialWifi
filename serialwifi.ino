/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*  EEPROM implementation test
/*****************************************************************************/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <EEPROM.h>
#include "webpage.h"
#include "FIFO.h"


static const size_t  KB	= 1024;						//1Kb = 1024bits
static const size_t  EEPROM_SIZE = 300;		//Max available size for the EEPROM data
static const size_t  ROM_BANK_SIZE = 30;			//size of each memory bank (10 banks: 0-9) 
static const double  VERSION = 1.25;

static const char*  wifiSSID	  = "(-_-)";
static const char*  wifiPassword  = "monteiro";
static const char*  hostName	  = "serialwifi";
static const char*  FaultFoundStr = "----> FAULT FOUND! <----";

String faultCommand;
String baudRateOption;
String dataFormatRadio;

MemoryBuffer dataBuffer;


ADC_MODE(ADC_VCC);									//needed to return voltage reading from ESP
ESP8266WebServer server(80);						//webserver default port
WebSocketsServer webSocket = WebSocketsServer(81);  //websocket default port

void handleRoot() {
	Serial.println("Handling request: Index..");

	String Index = FPSTR(HTTP_WEBSITE);		// get static webpage part residing in flash memory ~5Kb

	Index.replace("{{wifiSSID}}", String(wifiSSID));
	Index.replace("{{ipAddress}}", ipAddress());
	Index.replace("{{faultCommand}}", faultCommand);
	Index.replace("{{version}}", String(VERSION));
	Index.replace("{{" + baudRateOption + "}}", "selected");
	Index.replace("{{" + dataFormatRadio + "}}", "checked");

	server.sendHeader("Content-Length", String(Index.length()));
	server.send(200, "text/html", Index);
	Serial.println("Request sent!");
}

void handleNotFound() {
	Serial.println("Handling request: Not found");
	server.send(404, "text/plain", "404: Not found");
}

void EEPROM_SAVE(size_t BankNumber, String NewData) {

	BankNumber *= ROM_BANK_SIZE;
	char Data[ROM_BANK_SIZE];
	NewData.toCharArray(Data, ROM_BANK_SIZE);

	for (size_t i = 0; i < ROM_BANK_SIZE; i++) {
		EEPROM.write(BankNumber + i, Data[i]);
		//Serial.print(char(Data[i]));

		if (Data[i] == 0) break;
	}

	EEPROM.commit();
}

String EEPROM_READ(size_t BankNumber) {

	BankNumber *= ROM_BANK_SIZE;
	char Data[ROM_BANK_SIZE];

	for (size_t i = 0; i < ROM_BANK_SIZE; i++) {
		Data[i] = EEPROM.read(BankNumber + i);
		//Serial.print(char(Data[i]));

		if (Data[i] == 0) break;
	}

	return String(Data);
}

boolean romIsEmpty() {
	if (EEPROM_READ(0) == "ROM_OK") {
		return false;
		}
	else {
		return true;
		}
}

String ipAddress() {
	return WiFi.localIP().toString();
}

String powerSupply() {
	return String((float)ESP.getVcc() / KB);
}

void setSerialBaudrate(String option) {

	option.remove(0,6);				//remove text part of the string e.g.'option'
	size_t baud = option.toInt();	//and convert the remaining number to an integer 

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
	Serial.println("setting baud: " + String(baud));
	Serial.flush();
	Serial.begin(baud, SERIAL_8N1);
}

void sendWebSocketHexString(String NewData) {
	String dataPacket;

	dataPacket += String((float)dataBuffer.GetCurrentSize() / KB);
	dataPacket += ";";
	dataPacket += String((float)ESP.getFreeHeap() / KB);
	dataPacket += ";";
	dataPacket += powerSupply();
	dataPacket += ";";

	webSocket.broadcastTXT(dataPacket + NewData);

}

void sendWebSocketTextString(String NewData) {

	String tempStr;

	for (size_t Index = 0; Index < NewData.length(); Index++) {
		tempStr = String(NewData.charAt(Index), HEX);
		if (tempStr.length() < 2) {					//because e.g.'0a' will be returned as 'a'
			tempStr = "0" + tempStr;
		}
		sendWebSocketHexString(tempStr);
	}
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { //handle incoming data from the websocket

	if (type == WStype_TEXT){
		if (lenght) {
			Serial.println("Saving server data..");
			
			String webConfigs = PtrToString(payload);

			faultCommand = extractField(webConfigs, 0);			//text2
			baudRateOption = extractField(webConfigs, 1);		//option
			dataFormatRadio = extractField(webConfigs, 2);		//radio

			setSerialBaudrate(baudRateOption);//refresh new baudrate

			//save values to EEPROM;
			EEPROM_SAVE(2, faultCommand);
			EEPROM_SAVE(3, baudRateOption);
			EEPROM_SAVE(4, dataFormatRadio);
			EEPROM_SAVE(0, "ROM_OK");

			Serial.println("Done saving. New data reloaded.");
		}
		else {
			Serial.println("Restarting..");
			webSocket.disconnect();
			server.close();
			ESP.reset();
		}
	}
}

String PtrToString(uint8_t *str) {
	byte *p;
	String result;
	p = str;
	while (*p) {
		result += char(*p);
		p++;
	}
	return result;
}

String extractField(String data, uint8_t field) {
	
	uint8_t i = 0;
	String result = "";

	for (uint8_t Index = 0; Index < data.length(); Index++) {
		if (data.charAt(Index) == ';') {
			i++;
			Index++;
		}
		if (i == field) {
			result += data.charAt(Index);
		}
	}
	return result;
}

void setup(void) {

	setSerialBaudrate("");		//set to default baudrate (115200_8N1)
	Serial.setTimeout(500);
	Serial.println("=== ESP-01 Restart ===");
	EEPROM.begin(EEPROM_SIZE);
	Serial.print("Eeprom Conf. Size: ");
	Serial.println(EEPROM_SIZE);
	

	if (romIsEmpty()){
		Serial.println("Rom Empty.");	//zero all variables
		faultCommand = "";
		baudRateOption = "";
		dataFormatRadio = "";
	}
		else{
		Serial.println("Rom not Empty.");//populate variables with each memory bank data
		faultCommand = EEPROM_READ(2);
		baudRateOption = EEPROM_READ(3);
		dataFormatRadio = EEPROM_READ(4);
		Serial.println(faultCommand);
		Serial.println(baudRateOption);
		Serial.println(dataFormatRadio);

		setSerialBaudrate(baudRateOption);//refresh new baudrate
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


	Serial.print("IP address: ");
	Serial.println(ipAddress());

	server.on("/", HTTP_GET, handleRoot);  // when client requests webpage
	server.onNotFound(handleNotFound);     // When client requests an unknown webpage

	server.begin();
	Serial.println("HTTP server started.");

	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	Serial.println("Websocket open.");
	sendWebSocketHexString("");
}

void loop(void) {

	delay(100); //allow serial buffer to fill up
	server.handleClient();
	webSocket.loop();
	sendWebSocketHexString("");

	while (Serial.available()) {

		String tempBuffer = Serial.readStringUntil('\n');

		if (faultCommand) {
			dataBuffer.WriteStringToBuffer(tempBuffer);
			if (tempBuffer.indexOf(faultCommand) >= 0)	//lookup for fault command
				sendWebSocketHexString(dataBuffer.ReadHexStringFromBuffer());
		}
		else {
				sendWebSocketTextString(tempBuffer);
		}
		yield();
	}
}