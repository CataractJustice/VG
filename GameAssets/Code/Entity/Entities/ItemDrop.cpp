#include "ItemDrop.h"
#include "Entity/Entity.h"
#include "GameNode/GameNode3D.h"
#include "Entity/Entities/Behaviour/ItemBehaviour.h"
#include "Entity/Entities/Behaviour/SelfDestruct.h"
#include "VoxelMaterials.h"
#include "MaterialDropRenderer.h"

bool ItemDrop::create(const void* data, unsigned int size, Entity* entity, GameNode3D* node) const
{
	if(size <= sizeof(ItemDropDataHeader)) return false;
	const ItemDropDataHeader& header = *(const ItemDropDataHeader*)data;
	InventoryItem item;
	if(!item.loadBufferSafe(((const char*)data) + sizeof(header), size - sizeof(header)))
	{
		return false;
	}
	if(item.getId().starts_with(materialItemPrefix)) 
	{
		MaterialDropRenderer* renderer = new MaterialDropRenderer();
		renderer->materialId = VoxelMaterials::getInstance().getIdByName(VoxelMaterials::getInstance().parseMaterialNameFromMaterialItemId(item.getId()));
		node->attachComponent(renderer);
		node->getRelativeTransform().rotate(float(std::rand() % 128) / 128.0f * glm::pi<float>() * 2.0f, glm::ivec3(0.0f, 1.0f, 0.0f));
		node->getRelativeTransform().setScale(glm::vec3(0.42f));
	}
	ItemBehaviour* itemComp = new ItemBehaviour(item);
	node->attachComponent(itemComp);
	return true;
}

bool ItemDrop::create(const InventoryItem& item, glm::vec3 position, glm::vec3 velocity, Entity*& entity, GameNode3D*& node) const
{
	node = new GameNode3D();
	entity = new Entity(this->assignedId);
	SelfDestruct* sd = new SelfDestruct(60 * 60 * 5);
	ItemBehaviour* itemComponent = new ItemBehaviour(item);
	node->getRelativeTransform().setPosition(position);
	node->attachComponent(entity);
	node->attachComponent(sd);
	node->attachComponent(itemComponent);
	if(item.getId().starts_with(materialItemPrefix)) 
	{
		MaterialDropRenderer* renderer = new MaterialDropRenderer();
		renderer->materialId = VoxelMaterials::getInstance().getIdByName(VoxelMaterials::getInstance().parseMaterialNameFromMaterialItemId(item.getId()));
		node->attachComponent(renderer);
		node->getRelativeTransform().rotate(float(std::rand() % 128) / 128.0f * glm::pi<float>() * 2.0f, glm::ivec3(0.0f, 1.0f, 0.0f));
		node->getRelativeTransform().setScale(glm::vec3(0.42f));
	}
	itemComponent->setVelocity(velocity);
	return true;
}

InventoryItem& ItemDrop::getItemStack(Entity* entity) 
{
	return getItemComponent(entity)->getItem();
}

const InventoryItem& ItemDrop::getItemStack(const Entity* entity) 
{
	return getItemComponent(entity)->getItem();
}

ItemBehaviour* ItemDrop::getItemComponent(Entity* entity) 
{
	return entity->getParent()->findComponentsByType<ItemBehaviour>()[0];
}

const ItemBehaviour* ItemDrop::getItemComponent(const Entity* entity)
{
	return entity->getParentConst()->findComponentsByTypeConst<ItemBehaviour>()[0];
}

std::vector<char> ItemDrop::saveBuffer(const Entity* entity) const
{
	ItemDropDataHeader header;
	header.position = entity->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition();
	header.velocity = getItemComponent(entity)->getVelocity();
	std::vector<char> itemBuffer = getItemStack(entity).saveBuffer();
	std::vector<char> buffer(sizeof(header) + itemBuffer.size());
	memcpy(buffer.data(), &header, sizeof(header));
	memcpy(buffer.data() + sizeof(header), itemBuffer.data(), itemBuffer.size());
	return buffer;
}