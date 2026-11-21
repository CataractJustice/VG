#include "Player/PlayerManager.h"
#include "System/Logger.h"
void PlayerManager::handleSaveRequestHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(players.find(peer.id) == players.cend()) 
	{
		Logger::logInfo(std::format("Peer id {} requested save data before logging in.", peer.id));
		return;
	}
	Logger::logInfo(std::format("Got player save data request from peer id {}.", peer.id));
	sendPlayerSave(peer.id);
	return;
}