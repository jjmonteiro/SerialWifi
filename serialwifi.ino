/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*****************************************************************************/

#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <EEPROM.h>
#include "WEBPAGE.h"
#include "FIFO.h"


static const size_t  KB = 1024;				//1Kb = 1024bits
static const size_t  EEPROM_SIZE = 300;		//Max available size for the EEPROM data
static const size_t  ROM_BANK_SIZE = 30;	//size of each memory bank (10 banks: 0-9) 

static const char*  hostName = "serialwifi";

String faultCommand;
String baudRateOption;
String dataFormatRadio;
String ssid;
String psk;
MemoryBuffer dataBuffer;

ADC_MODE(ADC_VCC);									//needed to return voltage reading from ESP
ESP8266WebServer webServer(80);						//webserver default port
WebSocketsServer webSocket = WebSocketsServer(81);  //websocket default port

void handleRoot() {
	Serial.println("Handling request: Index..");

	String Index = FPSTR(HTTP_WEBSITE);		// get static webpage part residing in flash memory ~5Kb

	Index.replace("{{wifiSSID}}", String(WiFi.SSID()));
	Index.replace("{{ipAddress}}", WiFi.localIP().toString());
	Index.replace("{{faultCommand}}", faultCommand);
	Index.replace("{{" + baudRateOption + "}}", "selected");
	Index.replace("{{" + dataFormatRadio + "}}", "checked");

	webServer.sendHeader("Content-Length", String(Index.length()));
	webServer.send(200, "text/html", Index);
	Serial.println("Request sent!");
}

void handleNotFound() {
	Serial.println("Handling request: Not found");
	webServer.send(404, "text/plain", "404: Not found");
}

void EEPROM_WRITE(size_t BankNumber, String NewData) {

	BankNumber *= ROM_BANK_SIZE;
	char Data[ROM_BANK_SIZE];
	NewData.toCharArray(Data, ROM_BANK_SIZE);

	for (size_t i = 0; i < ROM_BANK_SIZE; i++) {
		EEPROM.write(BankNumber + i, Data[i]);
		if (Data[i] == 0) break;
	}

	EEPROM.commit();
}

String EEPROM_READ(size_t BankNumber) {

	BankNumber *= ROM_BANK_SIZE;
	char Data[ROM_BANK_SIZE];

	for (size_t i = 0; i < ROM_BANK_SIZE; i++) {
		Data[i] = EEPROM.read(BankNumber + i);
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

void setSerialBaudrate(String option) {

	option.remove(0, 6);				//remove text part of the string e.g.'option'
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

	dataPacket += String((float)dataBuffer.GetCurrentSize() / KB) + ";";
	dataPacket += String((float)ESP.getFreeHeap() / KB) + ";";
	dataPacket += String((float)ESP.getVcc() / KB) + ";";

	webSocket.broadcastTXT(dataPacket + NewData);
}

void sendWebSocketTextString(String NewData) {

	String HexChar;
	for (size_t Index = 0; Index < NewData.length(); Index++) {
		HexChar = String(NewData.charAt(Index), HEX);
		if (HexChar.length() < 2) {					//because e.g.'0a' will be returned as 'a'
			HexChar = "0" + HexChar;
		}
		sendWebSocketHexString(HexChar);
	}
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght) { //handle incoming data from the websocket

	if (type == WStype_TEXT) {
		if (lenght) {
			Serial.println("Saving server data..");

			String webConfigs = PtrToString(payload);
			Serial.println(webConfigs);

			faultCommand = extractField(webConfigs, 0);			//text2
			baudRateOption = extractField(webConfigs, 1);		//option
			dataFormatRadio = extractField(webConfigs, 2);		//radio

			setSerialBaudrate(baudRateOption);//refresh new baudrate

											  //save values to EEPROM;
			EEPROM_WRITE(2, faultCommand);
			EEPROM_WRITE(3, baudRateOption);
			EEPROM_WRITE(4, dataFormatRadio);
			EEPROM_WRITE(0, "ROM_OK");

			Serial.println("Done saving. New data reloaded.");
		}
		else {
			Serial.println("Restarting..");
			webSocket.disconnect();
			webServer.close();
			ESP.reset();
		}
	}
}

String PtrToString(uint8_t *str) {
	String result;
	byte *p = str;
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
	Serial.println();
	Serial.println("=== ESP-01 Restart ===");
	EEPROM.begin(EEPROM_SIZE);
	Serial.print("Eeprom Conf. Size: ");
	Serial.println(EEPROM_SIZE);


	if (romIsEmpty()) {
		Serial.println("Rom Empty.");	//zero all variables
		faultCommand = "";
		baudRateOption = "";
		dataFormatRadio = "";
	}
	else {
		Serial.println("Rom not Empty.");//populate variables with each memory bank data
		faultCommand = EEPROM_READ(2);
		baudRateOption = EEPROM_READ(3);
		dataFormatRadio = EEPROM_READ(4);
		ssid = EEPROM_READ(8);
		psk = EEPROM_READ(9);
		setSerialBaudrate(baudRateOption);//refresh new baudrate
	}

	ConnectWiFi();

	webServer.on("/", HTTP_GET, handleRoot);  // when client requests webpage
	webServer.onNotFound(handleNotFound);     // When client requests an unknown webpage

	webServer.begin();
	Serial.println("HTTP server started.");

	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	Serial.println("Websocket open.");
	sendWebSocketHexString("");
}

void ConnectWiFi() {
	Serial.println("Connecting WiFi..");
	WiFi.hostname(hostName);

	Serial.println("Attempting last known credentials..");
	WiFi.begin(ssid.c_str(), psk.c_str());	//try last known credentials

	if (WiFi.status() != WL_CONNECTED) {	//attempt #1
		for (size_t attempt = 0; (WiFi.status() != WL_CONNECTED && attempt < 10); attempt++) {
			delay(1000);
			Serial.print(".");
		}
	}

	if (WiFi.status() != WL_CONNECTED) {	//attempt #2
		Serial.println("Failed.");
		Serial.println("Attempting WPS connection..");
		WiFi.beginWPSConfig();
		for (size_t attempt = 0; (WiFi.status() != WL_CONNECTED && attempt < 120); attempt++) {
			delay(4000);
			Serial.print(".");
		}
	}

	if (WiFi.status() != WL_CONNECTED) {	//give up
		Serial.println("Failed. Restarting..");
		ESP.restart();
	}

	Serial.println();
	Serial.println("Connected!");
	EEPROM_WRITE(8, String(WiFi.SSID()));
	EEPROM_WRITE(9, String(WiFi.psk()));
	Serial.println(WiFi.localIP());
	Serial.println(WiFi.SSID());
	Serial.println(WiFi.macAddress());
}

void loop(void) {

	size_t loopcounter = 0;

	while (true) {
		webServer.handleClient();
		webSocket.loop();

		if (loopcounter > 10000) { //update values ~200ms
			sendWebSocketHexString("");
			loopcounter = 0;
		}

		while (Serial.available()) {

			String tempBuffer = Serial.readStringUntil('\n');

			if (faultCommand) {
				dataBuffer.WriteStringToBuffer(tempBuffer);
				if (tempBuffer.indexOf(faultCommand) >= 0) {//lookup for fault command
					if (dataBuffer.IsBufferFull)
						sendWebSocketHexString(dataBuffer.ReadStringFromRange(dataBuffer.GetCurrentPosition(), BUFFER_SIZE));
					if (dataBuffer.GetCurrentPosition())
						sendWebSocketHexString(dataBuffer.ReadStringFromRange(0, dataBuffer.GetCurrentPosition()));
					dataBuffer.ResetCurrentPosition();
				}
			}
			else {
				sendWebSocketTextString(tempBuffer);
			}
			yield();
		}
		loopcounter++;
	}
}