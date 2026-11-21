#include "JoinServerButton.h"
#include "Core.h"
#include "GameNode/ComponentFactory.h"
#include "../Menu/MenuInit.h"
#include "System/Logger.h"

JoinServerButton::JoinServerButton() 
{
	
}


void JoinServerButton::onClick(unsigned short button) 
{
	const std::string address = MenuInit::getMenuInstance()->getInputValue("address");
	std::string ip = "";
	unsigned int octet = 0;
	unsigned int port = 0;
	unsigned int power = 1;
	unsigned int num = 0;
	bool isPort = true;
	bool hasPort = false;
	bool isValid = true;
	
	for(int i = address.length() - 1; i >= 0; i--) 
	{
		if(address[i] == ':') 
		{
			if(!isPort) 
			{
				isValid = false;
				break;
			}
			if(num > 65535) 
			{
				isValid = false;
				break;
			}
			hasPort = true;
			port = num;
			power = 1;
			num = 0;
			isPort = false;
		}
		else if(address[i] == '.') 
		{
			if(num > 255 || power == 1) 
			{
				isValid = false;
				break;
			}
			isPort = false;
			octet++;
			if(octet > 4) 
			{
				isValid = false;
				break;
			}
			power = 1;
			num = 0;
		}
		else 
		{
 			int digit = address[i] - '0';
			if(digit < 0 || digit > 9) 
			{
				isValid = false;
				break;
			}
			num += digit * power;
			power *= 10;
			if(power > 100000000) 
			{
				isValid = false;
				break;
			}
			if(!isPort && power > 1000) 
			{
				isValid = false;
				break;
			}
		}
	}
	if(num > 255 || power == 1) 
	{
		isValid = false;
	}
	if(octet != 3) 
	{
		isValid = false;
	}
	if(!hasPort) 
	{
		port = 38390;
		ip = address;
	}
	else 
	{
		for(const char* c = address.data(); *c != ':'; c++) 
		{
			ip += *c;
		}
	}
	if(isValid) 
	{
		Logger::logInfo(std::format("Trying to connect to host {} at port {}", ip, port));
		Core::client.connect(ip, port);
	}
}

void JoinServerButton::onClientConnect() 
{
	Logger::logInfo("Connected to host, loading world");
	Core::setScene("./GameAssets/World.node");
}
void JoinServerButton::onClientConnectFail() 
{
	Logger::logInfo("Failed to connect to host");
}
void JoinServerButton::onClientDisconnect() 
{
	Logger::logInfo("Connection closed");
}
EXPORT_COMPONENT(JoinServerButton);	