#pragma once
#include "Entity/EntityTemplate.h"
class InventoryItem;
class Entity;
class ItemBehaviour;
#pragma pack(push, 1)
struct ItemDropDataHeader
{
	glm::vec3 position;
	glm::vec3 velocity;
};
#pragma pack(pop)
class ItemDrop : public EntityTemplate 
{
	public:
	bool create(const void* data, unsigned int size, Entity* entity, GameNode3D* node) const override;
	bool create(const InventoryItem& fromItem, glm::vec3 atPosition, glm::vec3 withVelocity, Entity*& entity, GameNode3D*& node) const;
	virtual std::vector<char> saveBuffer(const Entity* entity) const override;
	static InventoryItem& getItemStack(Entity* entity);
	static const InventoryItem& getItemStack(const Entity* entity);
	static ItemBehaviour* getItemComponent(Entity* entity);
	static const ItemBehaviour* getItemComponent(const Entity* entity);
};
