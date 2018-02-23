#pragma once
#include "IDeviceServer.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <iostream>


class ES8266Server : public IDeviceServer
{
	ESP8266WebServer &NativaServer;
public:
	ES8266Server():NativaServer(PORT)
	{
		
	}
	void  Send(const std::string & data, const std::string& contentType, unsigned int HttpCode)
	{
		NativaServer.send(HttpCode, contentType.c_str(), data.c_str());
	}
};


class MockServerServer : public IDeviceServer
{
	
public:
	MockServerServer()
	{

	}
	void  Send(const std::string & data, const std::string& contentType, unsigned int HttpCode)
	{
		std::cout << data <<" : Sent";
	}
};


 