#include "Entity/EntityManager.h"
#include "Entity/Packets/EntityDataPacket.h"
#include "Entity/Packets/Types.h"
#include "Core.h"
using namespace TealEngine;
void EntityManager::handleEntityDataPacketClient(const void* data, int size) 
{
	if(size <= sizeof(EntityDataPacket)) return;
	const EntityDataPacket& packet = *(const EntityDataPacket*)data;
	auto it = entities.find(packet.entityId);
	if(it == entities.cend()) 
	{
		this->createEntity(((const char*)data) + sizeof(EntityDataPacket), size - sizeof(EntityDataPacket), packet.entityId);
	}
	else 
	{
		it->second->loadBuffer((const char*)data, size);
	}
}