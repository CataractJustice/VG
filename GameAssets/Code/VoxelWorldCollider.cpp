#include "VoxelWorldCollider.h"
#include "GameNode/ComponentFactory.h"

void VoxelWorldCollider::onAttach() 
{
	this->world = this->getParent()->findComponentsByType<World>()[0];
}

float VoxelWorldCollider::getScalarFieldAt(glm::vec3 position) 
{
	if(world) 
	{
		return world->getValueAt(position);	
	}
	else 
	{
		return -1.0f;
	}
}

glm::vec3 VoxelWorldCollider::getNormalAt(glm::vec3 position) 
{
	if(world) 
	{
		return world->getNormalAt(position);	
	}
	else 
	{
		return glm::vec3(0.0f, 1.0f, 0.0f);
	}
}


EXPORT_COMPONENT(VoxelWorldCollider);
