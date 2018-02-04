/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*  EEPROM implementation test
/*****************************************************************************/

#define KB		 1024
#define PORT	 80
#define SERIAL_BAUDRATE 115200
#define MAX_LINES 40
#define MAX_CHARS 100

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
char emailAddress	[25];
char faultCommand	[15];

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
			"<input class='textbox border' type='email' placeholder='Email Address' maxlength='20' name='text1' value='" + 
			emailAddress + "' required>" +
			"<input class='textbox border' type='text' pattern='[0-9a-fA-F]{4,8}' placeholder='Hex Command' maxlength='10' name='text2' value='" + 
			faultCommand + "' required>" +
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
	server.arg("text1").toCharArray(emailAddress,25);
	server.arg("text2").toCharArray(faultCommand,15);
	Serial.println("Saving server data..");

	Serial.println("email:" + String(emailAddress));
	Serial.println("command:" + String(faultCommand));
	//save values to ROM;

	server.sendHeader("Location", "/", true); //redirect to prevent resubmission
	server.send(302, "text/plain", "");

	delay(300);
	ESP.restart();
}

void setup(void) {

	Serial.begin(SERIAL_BAUDRATE);
	WiFi.hostname(hostName);
	WiFi.begin(wifiSSID, wifiPassword);

	Serial.println();
	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	 
	WiFi.localIP().toString().toCharArray(ipAddress,16);
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
