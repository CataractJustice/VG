#include "Entity/EntityManager.h"
#include "Entity/Packets/EntityDataRequestPacket.h"
#include "Entity/Packets/EntityDataPacket.h"
#include "Entity/Packets/Types.h"
#include "Core.h"
using namespace TealEngine;
void EntityManager::handleEntityDataRequestPacketHost(PeerInfo& peer, const void* data, int size) 
{
	if(size != sizeof(EntityDataRequestPacket)) return;
	const EntityDataRequestPacket& packet = *(const EntityDataRequestPacket*)data;
	auto it = this->entities.find(packet.entityId);
	if(it == entities.cend()) return;
	EntityDataPacket response;
	response.entityId = packet.entityId;
	response.packetType = EntityManagerPacketTypes::entityData;
	response.entityType = it->second->getEntityTemplateId();
	std::vector<char> entityData = it->second->saveBuffer();
	std::vector<char> buffer(sizeof(EntityDataPacket) + entityData.size());
	memcpy(buffer.data(), &response, sizeof(response));
	memcpy(buffer.data() + sizeof(response), entityData.data(), entityData.size());
	Core::host.send(this->getNetworkId(), peer, buffer.data(), buffer.size());
}