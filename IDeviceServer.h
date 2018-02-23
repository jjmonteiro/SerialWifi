#ifndef DEVICE_CONNECTOR_H
#define DEVICE_CONNECTOR_H
#include <string>


class IDeviceServer
{
public:
	void virtual Send(const std::string & data, const std::string& contentType,unsigned int HttpCode) = 0;
};

#endif