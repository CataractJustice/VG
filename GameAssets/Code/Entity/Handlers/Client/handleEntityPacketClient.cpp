#include "Entity/EntityManager.h"
#include "Entity/Packets/EntityPacket.h"
#include "Entity/Packets/EntityDataRequestPacket.h"
#include "Entity/Packets/Types.h"
#include "Entity/Entities/Behaviour/EntityNetworkListener.h"
#include "Core.h"
using namespace TealEngine;
void EntityManager::handleEntityPacketClient(const void* data, int size) 
{
	if(size <= sizeof(EntityPacket)) return;
	const EntityPacket& packet = *(const EntityPacket*)data;
	auto it = entities.find(packet.entityId);
	if(it == entities.cend()) 
	{
		EntityDataRequestPacket request;
		request.entityId = packet.entityId;
		request.packetType = EntityManagerPacketTypes::entityDataRequest;
		Core::client.send(this->getNetworkId(), &request, sizeof(request));
		return;
	}
	const std::vector<EntityNetworkListener*>& enls = it->second->getParent()->findComponentsByType<EntityNetworkListener>();
	for(EntityNetworkListener* enl : enls) 
	{
		enl->onClientEntityPacket(((const char*)data) + sizeof(packet), size - sizeof(packet));
	}
}