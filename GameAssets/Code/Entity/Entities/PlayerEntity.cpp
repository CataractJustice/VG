#include "PlayerEntity.h"
#include "Entity/Entity.h"
#include "Player/PlayerBuilder.h"

bool PlayerEntity::create(const void* data, unsigned int size, Entity* node, GameNode3D* entity) const 
{
	return true;
}
std::vector<char> PlayerEntity::saveBuffer(const Entity* entity) const 
{
	return {};
}
PlayerBuilder* PlayerEntity::getPlayerBuilder(Entity* playerEntity) 
{
	return playerEntity->getParent()->getParent()->findComponentsByType<PlayerBuilder>()[0];
}