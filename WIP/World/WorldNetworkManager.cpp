#include "World.h"
#include "WorldNetworkManager.h"
#include "Packets/PacketTypes.h"

WorldNetworkManager::WorldNetworkManager(World* world) : world(world) 
{
	
}

void WorldNetworkManager::onClientPacket(const void* data, int size) 
{
	if(size == 0) return;
	unsigned char packetType = *(const unsigned char*)data;
	switch (packetType)
	{
	case WorldNetworkPacketTypes::chunkData:

		break;
	case WorldNetworkPacketTypes::chunkCache:

		break;
	case WorldPacketType::WorldInfo:
		if(size != sizeof(WorldInfoPacket)) return;
		
		{
			WorldInfoPacket& wip = *(WorldInfoPacket*)data;
			for(int i = 0; i < worldIdSize; i++) 
			{
				const char c = wip.id[i];
				if((c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '.' && c != '-' && c != '_') 
				{
					Logger::logInfo("Server sent invalid world id format.");
					return;
				}
			}
			worldName = std::string((const char*)wip.id, worldIdSize);
			memcpy(worldData.id, wip.id, worldIdSize);
			gotWorldInfo = true;
			Logger::logInfo(std::format("Got valid world info packet, world id: {}", worldName));
		}
	break;
	case WorldPacketType::SetCubeTile:
		if(!joinComplete) return;
		if(size != sizeof(SetCubeTilePacket)) return;
		{
			SetCubeTilePacket packet = *(const SetCubeTilePacket*)data;
			TileMap::TileData tile;
			tile.cubeMaterialId = packet.cubeId;
			tile.orientation = 0;
			tile.tileId = 0;
			setTileAt(glm::vec3(packet.x, packet.y, packet.z), tile);
		}
		break;
	case WorldPacketType::SetVoxel:
		if(!joinComplete) return;
		if(size != sizeof(SetVoxelPacket)) return;
		{
			SetVoxelPacket packet = *(const SetVoxelPacket*)data;
			Voxel voxel;
			voxel.voxelTypeId = packet.id;
			voxel.amount = packet.amount;
			setVoxel(glm::ivec3(packet.x, packet.y, packet.z), voxel);
		}
		break;
		return;
		break;
	}
}
void World::onHostPeerPacket(PeerInfo& peer, const void* data, int size) 
{
	if(size < sizeof(WorldPacketHeader)) return;
	switch ((WorldPacketType)((WorldPacketHeader*)(data))->packetType)
	{
	case WorldPacketType::RequestChunk:
		if(size != sizeof(ChunkRequestPacket)) return;
		{
			ChunkRequestPacket& packet = *((ChunkRequestPacket*)data);
			if(logChunkPackets) Logger::logInfo(std::format("Chunk {}.{}.{} requested.", packet.x, packet.y, packet.z));
			((chunkReqQueueT*)(chunkRequestsQueue))->emplace(std::pair<PeerInfo, ChunkRequestPacket>(peer, *((ChunkRequestPacket*)data)));
		}
		break;
	case WorldPacketType::WorldInfoRequest:
		{
			WorldInfoPacket packet;
			memcpy(packet.id, worldData.id, worldIdSize);
			Core::host.send({Core::networkDictionary.getNetIdOf("World"), 0}, peer, &packet, sizeof(packet));
		}
		break;
	default:
		return;
		break;
	}
}