#include "Player/PlayerManager.h"
#include "System/Logger.h"

void PlayerManager::handleAuthResposeClient(const void* data, unsigned int size) 
{
	if(sizeof(PlayerAuthenrificateResponsePacket) != size) return;
	const PlayerAuthenrificateResponsePacket& packet = *(PlayerAuthenrificateResponsePacket*)(data);
	if(packet.reject) 
	{
		Logger::logInfo(std::format("Authentification rejected, reject code {}.", packet.reject));
	}
	else 
	{
		Logger::logInfo(std::format("Authentification success.", packet.reject));
		authentificated = true;
	}
}