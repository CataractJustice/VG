#include "World/WorldNetworkManager.h"
#include "Packets/ChunkDataPacket.h"
#include "WorldManager.h"
#include "System/Logger.h"
#include "WorldStorage.h"
#include "WorldChunkManager.h"
#include <fstream>
#include <format>

void WorldNetworkManager::handleChunkDataClient(void* data, int size) 
{
	if(size < sizeof(ChunkDataPacket)) return;
	ChunkDataPacket& cdp = *(ChunkDataPacket*)data;
	Logger::logInfo(std::format("Got chunk {}.{}.{} data packet.", cdp.x, cdp.y, cdp.z));
	char* chunkDataBytes = ((char*)data) + sizeof(ChunkDataPacket);
	std::filesystem::path path = world->getStorage()->getChunkPath(glm::ivec3(cdp.x, cdp.y, cdp.z));
	std::ofstream file(path, std::ios::binary);
	file.write(chunkDataBytes, size - sizeof(ChunkDataPacket));
	world->getChunkManager()->requestChunkLoadFromDisc(glm::ivec3(cdp.x, cdp.y, cdp.z));
}