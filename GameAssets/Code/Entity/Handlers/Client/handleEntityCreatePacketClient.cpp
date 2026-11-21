#include "Entity/EntityManager.h"
#include "Entity/Packets/EntityCreatePacket.h"
#include "Entity/Packets/Types.h"
#include "Core.h"
using namespace TealEngine;
void EntityManager::handleEntityCreatePacketClient(const void* data, int size) 
{
	if(size <= sizeof(EntityCreatePacket)) return;
	const EntityCreatePacket& packet = *(const EntityCreatePacket*)data;
	this->createEntity(((const char*)data) + sizeof(EntityCreatePacket), size - sizeof(EntityCreatePacket), packet.entityId);
}