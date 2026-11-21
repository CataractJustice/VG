#include "OrbiterAI.h"
#include "GameNode/Properties/GameNodeProp.h"
#include "GameNode/ComponentFactory.h"
#include "World.h"
OrbiterAI::OrbiterAI() 
{
	this->speed = 0.1f;
	this->turning = 0.2f;
	this->target = nullptr;
	this->addProp(new FloatProp(&speed), "Speed");
	this->addProp(new FloatProp(&turning), "Turn");
}

void OrbiterAI::update() 
{
	Transform& transform = this->getParentOfType<GameNode3D>()->getRelativeTransform();
	transform.translate(transform.forward() * speed);
	if(!World::currentWorld) return;
	target = World::currentWorld->playerBuilder->getParent();
	if(!target) return;
	glm::vec3 dir = ((GameNode3D*)(target))->getWorldTransform().getPosition() - transform.getPosition();
	if(dir.x == 0.0f && dir.y == 0.0f && dir.z == 0.0f) return;	
	glm::vec3 turnAxis = glm::cross(transform.forward(), dir);
	if(std::isnan(turnAxis.x) || std::isnan(turnAxis.y) || std::isnan(turnAxis.z)) return;
	float angleDeviation = glm::acos(glm::dot(glm::normalize(dir), transform.forward()));
	if(std::isnan(angleDeviation)) return;
	transform.globalRotate(glm::min(angleDeviation, turning), turnAxis);
}

EXPORT_COMPONENT(OrbiterAI);