#include "EntityManager.h"
#include "EntityManagerPackets.h"
#include "System/Logger.h"
#include "EntityTemplates.h"
#include "EntityTemplate.h"
#include <fstream>
#include "World.h"

const char entityIdCounterPath[] = "./data/eic";

EntityManager::EntityManager() 
{
	nextEntityId = 0;
	if(std::filesystem::exists(entityIdCounterPath)) 
	{
		unsigned int size = std::filesystem::file_size(entityIdCounterPath);
		if(size != sizeof(unsigned long long)) 
		{
			std::filesystem::remove(entityIdCounterPath);
		}
		else 
		{
			std::ifstream entityIdCounterFile(entityIdCounterPath);
			entityIdCounterFile.read((char*)&nextEntityId, sizeof(nextEntityId));
			nextEntityId++;
		}
	}
}

unsigned long long EntityManager::getNextEntityId() 
{
	nextEntityId++;
	std::ofstream file(entityIdCounterPath);
	file.write((char*)&nextEntityId, sizeof(nextEntityId));
	return nextEntityId;
}

void EntityManager::onHostPeerPacket(PeerInfo& peer, const void* data, int size) 
{
	unsigned char packetType = *(const char*)data;
	switch(packetType) 
	{
		case EntityManagerPacketTypes::entityDataRequest: 
		handleEntityDataRequestPacketHost(peer, data, size);
		break;
		default:
		Logger::logInfo(std::format("Unknown EntityManger packet type id {}.", packetType));
	}
}

void EntityManager::onClientPacket(const void* data, int size) 
{
	unsigned char packetType = *(const char*)data;
	switch(packetType) 
	{
		case EntityManagerPacketTypes::entityCreate: 
		handleEntityCreatePacketClient(data, size);
		break;
		case EntityManagerPacketTypes::entityData: 
		handleEntityDataPacketClient(data, size);
		break;
		case EntityManagerPacketTypes::entityDestroy: 
		handleEntityDestroyPacketClient(data, size);
		break;
		case EntityManagerPacketTypes::entityPacket: 
		handleEntityPacketClient(data, size);
		break;
		default:
		Logger::logInfo(std::format("Unknown EntityManger packet type id {}.", packetType));
	}
}

//Client only
void EntityManager::createEntity(const void* data, int size, unsigned long long entityId) 
{
	Entity* entity = nullptr;
	GameNode3D* node = nullptr;
	bool created = EntityTemplates::getInstance()->createEntityFromBuffer(data, size, entity, node);
	if(!created) 
	{
		if(node) delete node;
		else if(entity) delete entity;
			throw std::runtime_error("Failed to create entity from buffer");
	}
	entity->assignId(entityId);
	auto it = entities.find(entityId);
	if(it != entities.cend()) 
	{
		destroyEntity(entityId);
	}
	entities[entityId] = entity;
	Core::getRoot()->addChild(node);
}
//Client only
void EntityManager::destroyEntity(unsigned long long id) 
{
	auto it = entities.find(id);
	if(it == entities.cend()) return;
	it->second->getParent()->destroy();
	entities.erase(id);
}

//Host only
void EntityManager::registerEntity(Entity* entity) 
{
	unsigned long long id = getNextEntityId();
	entities[id] = entity;
	entity->assignId(id);
	const auto& playersInRage = World::currentWorld->playerManager->getInRange(entity->getParentOfType<GameNode3D>()->getRelativeTransform().getPosition(), entitySyncRange);
	for(const auto[peerId, player] : playersInRage) 
	{
		PeerInfo peer;
		peer.id = peerId;
		EntityCreatePacket packet;
		packet.packetType = EntityManagerPacketTypes::entityCreate;
		packet.entityType = entity->getEntityTemplateId();
		packet.entityId = entity->getEntityId();
		const std::vector<char>& entityData = entity->saveBufferCommon();
		std::vector<char> buffer(sizeof(packet) + entityData.size());
		memcpy(buffer.data(), &packet, sizeof(packet));
		memcpy(buffer.data() + sizeof(packet),  entityData.data(), entityData.size());
		Core::host.send(getNetworkId(), peer, buffer.data(), buffer.size());
	}
}

void EntityManager::deregisterEntity(Entity* entity) 
{
	entities.erase(entity->getEntityId());
	const auto& playersInRage = World::currentWorld->playerManager->getInRange(entity->getParentOfType<GameNode3D>()->getRelativeTransform().getPosition(), entitySyncRange);
	for(const auto[peerId, player] : playersInRage) 
	{
		PeerInfo peer;
		peer.id = peerId;
		EntityDestroyPacket packet;
		packet.packetType = EntityManagerPacketTypes::entityDestroy;
		packet.entityId = entity->getEntityId();
		Core::host.send(getNetworkId(), peer, &packet, sizeof(packet));
	}
}