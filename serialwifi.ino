/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*  EEPROM implementation test
/*****************************************************************************/

#define KB		 1024
#define PORT	 80
#define SERIAL_BAUDRATE 115200
#define MAX_LINES 60
#define MAX_CHARS 100
#define EEPROM_SIZE 512		//Size can be anywhere between 4 and 4096 bytes
#define ROM_BANK_SIZE 40	//bytes
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

char ipAddress		[16];  //192.168.111.111
char freeHeap		[5];
char powerSupply	[5];
char emailAddress	[ROM_BANK_SIZE];
char faultCommand	[ROM_BANK_SIZE];

FIFO dataBuffer;

ADC_MODE(ADC_VCC);
ESP8266WebServer server(PORT);


void handleNotFound() {
	Serial.println("Handling request: Not found");
	server.send(404, "text/plain", "404: Not found");
}

void handleRoot() {
	String tmpBuffer;
	String Index_2;

	Serial.println("Handling request: index");

	dataBuffer.ReadAll(tmpBuffer);

	FPSTR(Index_1);

	Index_2 = "Network SSID: " + 
			String(wifiSSID) + "<br>" +
			"IP Address: " + 
			ipAddress + "<br>" +
			"Free Memory: " + 
			String((float)ESP.getFreeHeap() / KB) + "Kb<br>" +
			"Battery: " + 
			String((float)ESP.getVcc() / KB) + "V</p>" +
			"<p>Configurations</p>" +
			"<form action='/' method='POST' autocomplete='off' onsubmit=click1.value='Saving...'>" +
			"<input class='textbox border' type='email' placeholder='Email Address' maxlength='40' name='text1' value='" + 
			String(emailAddress) + "' required>" +
			"<input class='textbox border' type='text' pattern='[0-9a-fA-F]{4,8}' placeholder='Hex Command' maxlength='15' name='text2' value='" + 
			String(faultCommand) + "' required>" +
			"<input class='textbox border' type='submit' value='Save' name='click1'>" +
			"</form>" +
			"</nav>" +
			"<!---RIGHT PANEL--->" +
			"<article class='article'>" +
			"<p>Buffer Ascii Data</p>" +
			"<textarea class='border' readonly>" + 
			tmpBuffer +
			"</textarea>" +
			"</article>" +
			"<article class='article'>" +
			"<p>Buffer Byte Data</p>" +
			"<textarea class='border' readonly>" + 
			tmpBuffer +
			"</textarea>" +
			"</article>" +
			"<footer>Copyright &copy; 2018</footer>" +
			"</div>" + "</body>" + "</html>";

	server.send(200, "text/html",  Index_1 + Index_2);
}

void handleSave() {
	server.arg("text1").toCharArray(emailAddress, sizeof(emailAddress));
	server.arg("text2").toCharArray(faultCommand, sizeof(faultCommand));

	Serial.println("Saving server data..");
	Serial.println("email:" + String(emailAddress));
	Serial.println("command:" + String(faultCommand));
	//save values to ROM;	
	writeEeprom(emailAddress, 1);
	writeEeprom(faultCommand, 2);


	server.sendHeader("Location", "/", true); //redirect to prevent resubmission
	server.send(302, "text/plain", "");


	//ESP.restart();
}

void writeEeprom(char* NewData, int BankNumber) {

	Serial.println("Writting to EEPROM");
	BankNumber *= ROM_BANK_SIZE;

	for (int i = 0; i < ROM_BANK_SIZE; i++) {
		EEPROM.write(BankNumber + i, NewData[i]);
		Serial.print(char(NewData[i]));
		
		if (NewData[i] == '\0') return;
	}

	Serial.println();
	EEPROM.commit();
	//EEPROM.end();
}

void readEeprom(char* NewData, int BankNumber) {

	Serial.println("Reading from EEPROM");
	BankNumber *= ROM_BANK_SIZE;

	for (int i = 0; i < ROM_BANK_SIZE; i++){
		NewData[i] = EEPROM.read(BankNumber + i);
		Serial.print(char(NewData[i]));

		if (NewData[i] == '\0') return;
	}

	Serial.println();
	//EEPROM.end();
}

void setup(void) {

	Serial.setTimeout(SERIAL_TIMEOUT);
	Serial.begin(SERIAL_BAUDRATE);
	EEPROM.begin(EEPROM_SIZE);

	readEeprom(emailAddress, 1);
	readEeprom(faultCommand, 2);

	WiFi.hostname(hostName);
	WiFi.begin(wifiSSID, wifiPassword);

	Serial.println();
	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(200);
		Serial.print(".");
	}

	 
	WiFi.localIP().toString().toCharArray(ipAddress, sizeof(ipAddress));
	dataBuffer.Init();

	Serial.print("Connected to ");
	Serial.println(wifiSSID);
	Serial.print("IP address: ");
	Serial.println(ipAddress);
	Serial.print("Netbios: ");
	Serial.println(hostName);


	server.on("/", HTTP_GET, handleRoot);  // when client requests webpage
	server.on("/", HTTP_POST, handleSave); // when client saves changes
	server.onNotFound(handleNotFound);     // When client requests an unknown webpage

	server.begin();
	Serial.println("HTTP server started");
}

void loop(void) {
	server.handleClient();

	if (Serial.available()){
		Serial.println("Data received, line: " + String(dataBuffer.nextLine));
		dataBuffer.WriteLine(Serial.readString());
	}

}
