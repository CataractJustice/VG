#include "WorldCubeColliders.h"
#include "VoxelMaterials.h"
#include "GameNode/ComponentFactory.h"
using namespace TealEngine;

void WorldCubeColliders::onAttach() 
{
	this->world = nullptr;
	auto worlds = this->getParent()->findComponentsByType<World>();
	if(worlds.size()) 
	{
		this->world = worlds[0];
	}
	boxCollider = new BoxCollider();
	boxCollider->setRemoveFromScene(true);
	this->getParent()->attachComponent(boxCollider);
}
std::vector<Collider*> WorldCubeColliders::getCollidersAt(glm::vec3 position) 
{
	if(world == nullptr) return {};
	if(!Component::isValidComponent(world)) return {};	
	TileMap::TileData t = world->getTileAt(position);
	if(t.cubeMaterialId) 
	{
		boxCollider->from = glm::floor(position) - tileWorldOffset;
		boxCollider->to = glm::floor(position) + 1.0f - tileWorldOffset;
		return {boxCollider};
	}
	return {};
}

EXPORT_COMPONENT(WorldCubeColliders);