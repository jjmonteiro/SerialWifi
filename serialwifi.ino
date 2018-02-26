/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*  EEPROM implementation test
/*****************************************************************************/

#define KB		 1024
#define PORT	 80
#define SERIAL_BAUDRATE 115200
#define MAX_LINES 70		//40*100 = 4KB
#define MAX_CHARS 100
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

MemoryBuffer dataBuffer;

String ipAddress;
String freeHeap;
String powerSupply;
String Index;
String tmpBuffer;

ADC_MODE(ADC_VCC);
ESP8266WebServer server(PORT);




void handleRoot() {


	Serial.println("Handling request: index..");
	tmpBuffer = dataBuffer.ReadStringFromBuffer();
	Serial.println("Buffer retrieved.");

	// get static webpage part residing in flash memory
	FPSTR(http);

	Serial.println("Index_1 retrieved.");
	//changeable webpage parts
	
	Index = 
				("Network SSID: " +
				String(wifiSSID) + "<br>" +
				"IP Address: " +
				ipAddress + "<br>" +
				"Free Memory: " +
				freeHeap + "Kb<br>" +
				"Battery: " +
				powerSupply + "V</p>" +
				"<p>Configurations</p>" +
				"<form action='/' method='POST' autocomplete='off' onsubmit=click1.value='Saving...'>" +
				"<input class='textbox border' type='email' placeholder='Email Address' maxlength='40' name='text1' value='" +
				emailAddress + "' required>" +
				"<input class='textbox border' type='text' placeholder='Lookup Command' maxlength='40' name='text2' value='" +
				faultCommand + "' required>" +
				"<input class='textbox border' type='submit' value='Save' name='click1'>" +
				"</form>" +
				"</nav>" +
				"<!---RIGHT PANEL--->" +
				"<article class='article'>" +
				"<p>Buffer Ascii Data</p>" +
				"<textarea class='border' readonly>" +
				tmpBuffer + "</textarea>" +
				"</article>" +
				"<footer>Copyright &copy; 2018 Joaquim Monteiro</footer>" +
				"</div>" + "</body>" + "</html>");
	
	Serial.println("Index_2 constructed.");
	Serial.println("Sending request..");
	server.send(200, "text/html", (http + Index));
	Serial.println("Request sent!");
	tmpBuffer = "";
	Index = "";
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
	Serial.begin(SERIAL_BAUDRATE);
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

	if (Serial.available()){
		freeHeap = String((float)ESP.getFreeHeap() / KB);
		powerSupply = String((float)ESP.getVcc() / KB);

		Serial.println("Data received, line: " + String(dataBuffer.CurrentBufferPosition));
		dataBuffer.WriteStringToBuffer(Serial.readString());
	}

	delay(100);
}
