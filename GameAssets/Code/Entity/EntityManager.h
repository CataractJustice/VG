#pragma once
#include "GameNode/Component.h"
#include "Entity.h"
using namespace TealEngine;

static const float entitySyncRange = 128.0f;

class EntityManager : public Component
{
	private:
	std::map<unsigned long long, Entity*> entities;
	unsigned long long nextEntityId;
	public:
	EntityManager();
	void onClientPacket(const void* data, int size) override;
	void onHostPeerPacket(PeerInfo& peer, const void* data, int size) override;

	void handleEntityCreatePacketClient(const void* data, int size);
	void handleEntityDataPacketClient(const void* data, int size);
	void handleEntityDestroyPacketClient(const void* data, int size);
	void handleEntityPacketClient(const void* data, int size);

	void handleEntityDataRequestPacketHost(PeerInfo& peer, const void* data, int size);
	//client side
	void createEntity(const void* data, int size, unsigned long long entityId);
	void destroyEntity(unsigned long long id);
	//server side
	void registerEntity(Entity*);
	void deregisterEntity(Entity*);
	
	unsigned long long getNextEntityId();
};