#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "webpage.h"
#include "FIFO.h"

const char* ssid = "(-_-)";
const char* password = "monteiro";
const char* netbios = "serialwifi";

String myIP;
String myMEM;
String myPWR;
String myEMAIL;
String myDATA;
char myBUF[100];

FIFO myBUFFER;


ADC_MODE(ADC_VCC);
ESP8266WebServer server(80);

String WEB_index(){
  myMEM = String((float)ESP.getFreeHeap()/1024);
  myPWR = String((float)ESP.getVcc()/1024.0);
  myBUFFER.read(myBUF);

  String Index_2 =
  "Network SSID: " + String(ssid) + "<br>" +
  "IP Address: " + myIP + "<br>" +
  "Free Memory: " + myMEM + "Kb<br>" +
  "Battery: " + myPWR + "V</p>" +
  "<p>Configurations</p>" +
  "<form action='/' method='POST' autocomplete='off' onsubmit=click1.value='Saving...'>" +
  "<input class='textbox border' type='email' placeholder='Email Address' maxlength='20' name='text1' value='" + myEMAIL + "' required>" +
  "<input class='textbox border' type='text' pattern='[0-9a-fA-F]{4,8}' placeholder='Hex Command' maxlength='10' name='text2' value='" + myDATA + "' required>" +
  "<input class='textbox border' type='submit' value='Save' name='click1'>" +
  "</form>" +
  "</nav>" +
  "<!---RIGHT PANEL--->" +
  "<article class='article'>" +
  "<p>Buffered Data</p>" +
  "<textarea class='border' readonly>" + String(myBUF) + "</textarea>" +
  "</article>" +
  "<footer>Copyright &copy; 2018</footer>" +
  "</div>" + "</body>" + "</html>";

  return String(Index_1) + Index_2;
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

void handleRoot() {
  server.send(200, "text/html", WEB_index());
}

void handleSave() {
    myEMAIL = server.arg("text1");
    myDATA = server.arg("text2");

    Serial.println("email:" + myEMAIL);
    Serial.println("command:" + myDATA);
    //save values to ROM;
    //ESP.restart();
    delay(1000);
    server.sendHeader("Location", String("/"), true); //redirect to prevent resubmission
    server.send (302, "text/plain", "");
}

void setup(void){

  Serial.begin(115200);
  WiFi.hostname(netbios);
  WiFi.begin(ssid, password);

  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  myIP = WiFi.localIP().toString();
  myBUFFER.init();

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(myIP);
  Serial.print("Netbios: ");
  Serial.println(netbios);


  server.on("/", HTTP_GET, handleRoot);  // when client requests webpage
  server.on("/", HTTP_POST, handleSave); // when client saves changes
  server.onNotFound(handleNotFound);     // When client requests an unknown webpage

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();

  if (Serial.available() > 0) {
    myBUFFER.push(char(Serial.read()));
  }
}
