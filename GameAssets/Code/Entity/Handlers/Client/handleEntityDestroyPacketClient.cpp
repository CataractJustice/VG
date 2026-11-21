#include "Entity/EntityManager.h"
#include "Entity/Packets/EntityDestroyPacket.h"
#include "Entity/Packets/Types.h"
#include "Core.h"
using namespace TealEngine;
void EntityManager::handleEntityDestroyPacketClient(const void* data, int size) 
{
	if(size != sizeof(EntityDestroyPacket)) return;
	const EntityDestroyPacket& packet = *(const EntityDestroyPacket*)data;
	auto it = entities.find(packet.entityId);
	if(it == entities.cend()) 
	{
		return;
	}
	destroyEntity(packet.entityId);
}