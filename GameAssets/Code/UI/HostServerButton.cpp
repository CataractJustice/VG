#include "HostServerButton.h"
#include "Core.h"
#include "GameNode/ComponentFactory.h"
#include "../Menu/MenuInit.h"
#include "Menu.h"
#include "System/Logger.h"


void HostServerButton::onClick(unsigned short button) 
{
	const std::string portStr = MenuInit::getMenuInstance()->getInputValue("port");
	const unsigned int defaultPort = 38219;
	unsigned int port = 0;
	unsigned int maxConnections = 20;
	unsigned int p = 1;
	if(portStr.length() == 0) port = defaultPort;
	else
	for(int i = portStr.length() - 1; i >= 0; i--) 
	{
		const char c = portStr[i];
		if(c < '0' || c > '9' || port > 65535) 
		{
			port = defaultPort;
			break;
		}
		port += (c - '0') * p;
		p *= 10;
	}
	Logger::logInfo(std::format("Starting host at port {}, with {} max connections.", port, maxConnections));
	Core::host.host(port, maxConnections);
}

//called on a host every time a new client is connected
void HostServerButton::onHostPeerConnect(PeerInfo& peerInfo) 
{
	Logger::logInfo(std::format("New client connection id {}.", peerInfo.id));
	//Add connection to a registry and wait for authentification packets;
}
//called on a host every time a client disconnects
void HostServerButton::onHostPeerDisconnect(PeerInfo& peerInfo) 
{
	Logger::logInfo(std::format("Connection id {} closed.", peerInfo.id));
}
//called on a host every time a packet is received from a client
void HostServerButton::onHostPeerPacket(PeerInfo& peerInfo, const void* data, int size) 
{
	
}
EXPORT_COMPONENT(HostServerButton);	