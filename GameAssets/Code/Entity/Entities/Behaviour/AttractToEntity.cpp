/*#include "AttractToEntity.h"
#include "Entity/Entity.h"
#include "World.h"
#include "Math/IndexMath.h"
#include "Entity/EntityTools/getClosestEntityInRange.h"
#include "Entity/EntityTools/getEntitiesInRange.h"
AttractToEntity::AttractToEntity() 
{
	attractedTo = 0;
	impulsePerTicks = 0.0f;
	range = 1.0f;
	velocityPtr = nullptr;
	targetSelection = AttractionSelection::ChooseClosest;
	inverseFactor = 0.0f;
};

void AttractToEntity::update() 
{
	if(velocityPtr == nullptr) return;
	const float chunkSize = 32.0f;
	const float margins = 0.01f;
	const float offset = 0.01f;
	const glm::vec3 position = this->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition();
	std::vector<Entity*> attractingTo;
	if(targetSelection == AttractionSelection::ChooseClosest) 
	{
		attractingTo.push_back(getClosestEntityInRange(
			{attractedTo},
			position,
			range
		));
	}
	else if(targetSelection == AttractionSelection::Average) 
	{
		attractingTo = getEntitiesInRange(
			{attractedTo},
			position,
			range
		);
	}
	for(Entity* e : attractingTo) 
	{
		const glm::vec3 ePos = e->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition(); 
		const glm::vec3 delta = ePos - position;
		const float len = glm::length(delta);
		*this->velocityPtr += delta / len / (1.0f + len * inverseFactor);
	}
}*/