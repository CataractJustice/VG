#include "ItemBehaviour.h"
#include "Entity/EntityTools/getEntitiesInRange.h"
#include "Entity/EntityTools/getClosestEntityInRange.h"
#include "GameNode/GameNode3D.h"
#include "Entity/Entities/ItemDrop.h"
#include "Entity/Entity.h"
#include "Entity/EntityTemplates.h"
#include "Inventory/InventoryItem.h"
#include "World.h"
#include "Entity/Entities/PlayerEntity.h"
#include "Player/PlayerBuilder.h"
#include "Entity/Packets/EntityPacket.h"
#include "Entity/Packets/Types.h"

unsigned char mergeAttemptNextOffset = 0x61;

static unsigned int itemDropEntityType = ~0;
static unsigned int playerEntityType = ~0;

#pragma pack(push, 1)
struct ItemBehaviourPacketHost
{
	EntityPacket header;
	float x;
	float y;
	float z;
	float dx;
	float dy;
	float dz;
};

struct ItemBehaviourPacketClient 
{
	float x;
	float y;
	float z;
	float dx;
	float dy;
	float dz;
};
#pragma pack(pop)

ItemBehaviour::ItemBehaviour(const InventoryItem& item) : ticksExisting(0) 
{
	if(itemDropEntityType == ~0) 
	{
		itemDropEntityType = EntityTemplates::getInstance()->getTemplateId("item");
		playerEntityType = EntityTemplates::getInstance()->getTemplateId("player");
	}
	mergeAttemptTickOffset = mergeAttemptNextOffset;
	mergeAttemptNextOffset += 1;
	mergeAttemptNextOffset |= 0x61;
	this->item = item;
}

void ItemBehaviour::update() 
{
	//How much velocity item retains each tick while on ground
	const float dragFactor = 0.9f;
	//How much velocity item retains each tick while in air
	const float airDragFactor = 0.96f;
	//
	const float pushUpForce = 0.02f;
	const float gravityForce = 0.02f;
	const float castHeight = 0.8f;
	const float slopeThreshold = 0.7f;
	const float floorHardStopThreshold = 0.05f;
	const float slideForce = 0.01f;
	const float attractForce = 0.05f;
	const float inverseDistanceFactor = 0.1f;
	const float resyncDistance = 0.1f;
	const unsigned int ticksPerSync = 90;

	ticksExisting++;
	if(item.amount == 0) 
	{
		this->getParent()->destroy();
		return;
	}

	const glm::vec3 position = this->getParentOfType<GameNode3D>()->getWorldTransform().getPosition();
	const float pickRange = 1.2f;
	const float attractRange = 2.0f;

	if(Core::host.isLive() &&
		(
			glm::distance(position, lastSyncPosition) > resyncDistance ||
			ticksExisting % ticksPerSync == 0
		)
	) 
	{
		lastSyncPosition = position;
		const auto& players = World::currentWorld->playerManager->getInRange(position, entitySyncRange);
		for(const auto&[peerId, player] : players) 
		{
			PeerInfo peer;
			peer.id = peerId;
			ItemBehaviourPacketHost packet;
			packet.header.entityId = this->getParent()->findComponentsByType<Entity>()[0]->getEntityId();
			packet.header.packetType = EntityManagerPacketTypes::entityPacket;
			packet.x = position.x;
			packet.y = position.y;
			packet.z = position.z;
			packet.dx = velocity.x;
			packet.dy = velocity.y;
			packet.dz = velocity.z;
			Core::host.send(World::currentWorld->entityManager->getNetworkId(), peer, &packet, sizeof(packet));
		}
	}

	//player attraction and player picking
	if(Core::client.isConnected() == false) 
	{	
		std::map<float, Entity*> playerEntities = getEntitiesInRange({playerEntityType}, position, attractRange);
		for(auto[distance, playerEntity] : playerEntities) 
		{
			PlayerBuilder* pb = PlayerEntity::getPlayerBuilder(playerEntity);
			unsigned int pushStackSize = pb->inventory->pushStackCheck(item);
			if(pushStackSize == 0) continue;
			if(distance < pickRange) 
			{
				pb->inventory->pushStack(item);
				item.amount -= pushStackSize;
				if(item.amount == 0) 
				{
					this->getParent()->destroy();
					return;
				}
			}
			if(distance < 0.001f) continue;
			const glm::vec3 playerPosition = playerEntity->getParentOfType<GameNode3D>()->getWorldTransform().getPosition();
			const glm::vec3 playerPosDelta = playerPosition - position;
			this->velocity += playerPosDelta / distance / (1.0f + distance * inverseDistanceFactor) * attractForce;
		}
	}
	
	glm::vec3 hit = position;
	glm::vec3 hitNormal;
	glm::ivec3 hitIndex;
	bool onGround = World::currentWorld->rayCast(hit, glm::vec3(0.0f, -1.0f, 0.0f), 2, castHeight, hitIndex, hitNormal);
	velocity *= onGround ? dragFactor : airDragFactor;
	if(onGround) 
	{
		const float floorHeight = position.y - hit.y;
		velocity.y += (castHeight - floorHeight) / castHeight * pushUpForce;
		if(velocity.y < 0.0f && floorHeight < floorHardStopThreshold) 
		{
			velocity.y = 0.0f;
		}
	}
	else 
	{
		velocity.y -= gravityForce;
	}

	if(hitNormal.y < slopeThreshold && onGround) 
	{
		velocity += hitNormal * glm::vec3(1.0f, 0.0f, 1.0f) * slideForce;
	}

	if(World::currentWorld->getValueAt(position) > 0.0f) 
	{
		this->velocity += World::currentWorld->getNormalAt(position) * pushUpForce;
	}

	velocity = glm::clamp(velocity, -0.15f, 0.15f);

	this->getParentOfType<GameNode3D>()->getRelativeTransform().translate(velocity);

	this->getParentOfType<GameNode3D>()->getRelativeTransform().rotate(0.01f, glm::vec3(0.0f, 1.0f, 0.0f));

	if(Core::client.isConnected() == false) 
	{
		const unsigned int mergeAttemptTicksPerTry = 15;
		if((ticksExisting + mergeAttemptTickOffset) % mergeAttemptTicksPerTry) return;
		if(
			item.slots.size()
		) return;
		const float mergeRange = 1.8f;
		auto itemsInMergeRange = getEntitiesInRange({itemDropEntityType}, position, mergeRange);
		for(auto[distance, entity] : itemsInMergeRange) 
		{
			if(this == ItemDrop::getItemComponent(entity)) continue;
			ItemDrop* itemTemplate = ((ItemDrop*)EntityTemplates::getInstance()->getTemplate(entity->getEntityTemplateId()));
			InventoryItem& nearbyItem = itemTemplate->getItemComponent(entity)->item;
			//if item is empty it will be destroyed next frame, do not merge with it
			if(nearbyItem.amount == 0) continue;
			//Item has a different type
			if(nearbyItem.getTemplatePtr() != item.getTemplatePtr()) continue;
			//Do not merge with item that has slots
			unsigned int nearbyItemSlotsCount = nearbyItem.slots.size();
			if(nearbyItemSlotsCount != 0) continue;
			//Prefer merging into newer items
			if(this->ticksExisting < itemTemplate->getItemComponent(entity)->ticksExisting) continue;
			//Prefer merging into larger stacks
			if(this->ticksExisting == itemTemplate->getItemComponent(entity)->ticksExisting && this->item.amount > nearbyItem.amount) continue;
			//Merge
			nearbyItem.amount += item.amount;
			this->item.amount = 0;
			this->getParent()->destroy();
			return;
		}
	}
}

void ItemBehaviour::onClientEntityPacket(const void* data, unsigned int size) 
{
	if(size != sizeof(ItemBehaviourPacketClient)) return;
	const ItemBehaviourPacketClient& packet = *(const ItemBehaviourPacketClient*)data;
	this->getParentOfType<GameNode3D>()->getRelativeTransform().setPosition(glm::vec3(packet.x, packet.y, packet.z));
	this->velocity = glm::vec3(packet.dx, packet.dy, packet.dz);
}