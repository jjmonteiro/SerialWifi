/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*  EEPROM implementation test
/*****************************************************************************/

#define KB		 1024
#define PORT	 80
#define SERIAL_BAUDRATE 115200
#define MAX_LINES 70		//40*100 = 4KB
#define MAX_CHARS 100
#define EEPROM_SIZE 256		//Size can be anywhere between 4 and 4096 bytes
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

char emailAddress	[ROM_BANK_SIZE];
char faultCommand	[ROM_BANK_SIZE];

FIFO dataBuffer;
String tmpBuffer;
String ipAddress;
String freeHeap;
String powerSupply;

ADC_MODE(ADC_VCC);
ESP8266WebServer server(PORT);


void handleNotFound() {
	Serial.println("Handling request: Not found");
	server.send(404, "text/plain", "404: Not found");
}

void handleRoot() {

	Serial.println("Handling request: index..");

	dataBuffer.ReadAll(tmpBuffer);
	freeHeap = String((float)ESP.getFreeHeap() / KB);
	powerSupply = String((float)ESP.getVcc() / KB);
	Serial.println("Strings populated.");

	// get static webpage part residing in flash memory
	FPSTR(Index_1);
	Serial.println("Index_1 retrieved.");
	//changeable webpage parts
	String Index_2 =
				"Network SSID: " +
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
				String(emailAddress) + "' required>" +
				"<input class='textbox border' type='text' placeholder='Lookup Command' maxlength='40' name='text2' value='" +
				String(faultCommand) + "' required>" +
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
				"</div>" + "</body>" + "</html>";
	
	Serial.println("Index_2 constructed.");
	Serial.println("Sending request..");

	server.send(200, "text/html", (Index_1 + Index_2));
	Serial.println("Request sent!");
}

void handleSave() {
	server.arg("text1").toCharArray(emailAddress, ROM_BANK_SIZE);
	server.arg("text2").toCharArray(faultCommand, ROM_BANK_SIZE);

	Serial.println("Saving server data..");
	Serial.println("email:" + String(emailAddress));
	Serial.println("command:" + String(faultCommand));
	
	//save values to EEPROM;	
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
		
		if (NewData[i] == 0) break;
	}

	EEPROM.commit();
	Serial.println();
	Serial.println("Done writting.");
}

void readEeprom(char* NewData, int BankNumber) {

	Serial.println("Reading from EEPROM");
	BankNumber *= ROM_BANK_SIZE;

	for (int i = 0; i < ROM_BANK_SIZE; i++){
		NewData[i] = EEPROM.read(BankNumber + i);
		Serial.print(char(NewData[i]));

		if (NewData[i] == 0) break;
	}

	Serial.println();
	Serial.println("Done reading.");
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

	Serial.println("Reading Eeprom data..");
	readEeprom(emailAddress, 1);
	readEeprom(faultCommand, 2);

	Serial.println("Bank 1:" + String(emailAddress));
	Serial.println("Bank 2:" + String(faultCommand));


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

	dataBuffer.Init();
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
		Serial.println("Data received, line: " + String(dataBuffer.nextLine));
		dataBuffer.WriteLine(Serial.readString());
	}
	delay(300);
}
