#include "Player/PlayerManager.h"
#include "System/Logger.h"
#include "World.h"
#include <fstream>
void PlayerManager::handleSaveDataClient(const void* data, unsigned int size) 
{
	//early return if size is less than headers and save data is empty
	if((sizeof(PlayerSavePacket) + sizeof(PlayerSaveHeader)) >= size) 
	{
		Logger::logInfo(std::format("Server sent player save data that is too short, invalid headers. Packet size is {}.", size));
		return;
	}
	const PlayerSavePacket& savePacket = *((const PlayerSavePacket*)(data));
	const PlayerSaveHeader& saveHeader = *(const PlayerSaveHeader*)((const char*)(data) + sizeof(PlayerSavePacket));
	//size in header is inconsistent with packet size
	if(saveHeader.dataSize != (size - sizeof(PlayerSavePacket))) 
	{
		Logger::logInfo(std::format("Server sent player save data that has invalid data size in header, header value: {}; actual size: {}.", saveHeader.dataSize, (size - sizeof(PlayerSavePacket))));
		return;
	}
	std::ofstream saveFile = std::ofstream(World::currentWorld->getPlayersDirectory() / getUUID());
	saveFile.write((char*)(data) + sizeof(PlayerSavePacket), saveHeader.dataSize);
	createPlayer(savePacket.connectionId, getUUID(), PlayerBuilder::PlayerType::ControlledPlayer);
}