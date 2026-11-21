#include "GrabberArm.h"
#include "GameNode/ComponentFactory.h"
void GrabberArm::onAttach()
{
	const Transform& t = this->getParentOfTypeConst<GameNode3D>()->getWorldTransform();
	armTarget = t.getPosition() + t.forward() * radius;
	oldArmTarget = armTarget;
	interpolationFactor = 1.0f;
	armTargetInterpolated = armTarget;
	armTargetInterpolated = glm::vec3(0.0f);
	GameNode* absolute = new GameNode();
	this->getParent()->addChild(absolute);
	absolute->rename("Absolute");
	const std::vector<GameNode*>& childNodes = this->getParent()->getChilds();
	for(GameNode* child : childNodes) 
	{	
		GameNode3D* node3d = dynamic_cast<GameNode3D*>(child);
		if(node3d) 
		{
			armNodes.push_back(node3d);
		}
	}
	for(GameNode3D* armNode : armNodes) 
	{
		invKinematics.pushSegment(armNode, radius / armNodes.size());
		absolute->addChild(armNode);
	}
}

GrabberArm::GrabberArm()
{
	this->addProp(new FloatProp(&radius), "Arm radius");
	this->addProp(new FloatProp(&stepRadius), "Step radius");
	this->addProp(new FloatProp(&interpolationSpeed), "Arm speed");
	this->addProp(new FloatProp(&stepHeight), "Step height");
	this->addProp(new FloatVecProp(glm::value_ptr(castOffset), 3), "Cast offset");
}

void GrabberArm::update()
{
	const Transform& t = this->getParentOfType<GameNode3D>()->getWorldTransform();
	glm::vec3 castOrigin = t.getPosition() + t.right() * castOffset.x + t.up() * castOffset.y + t.forward() * castOffset.z;
	glm::vec3 castDirection = t.forward();
	const glm::vec3 originTargetDelta = armTarget - t.getPosition();
	const glm::vec3 castOriginTargetDelta = armTarget - castOrigin;
	const glm::vec3 stepFlatDelta = castOriginTargetDelta - castDirection * glm::dot(castDirection, castOriginTargetDelta);
	const float stepDistance = glm::length(stepFlatDelta);
	if(glm::length(originTargetDelta) > radius || stepDistance > stepRadius) 
	{
		oldArmTarget = armTargetInterpolated;
		armTarget = castOrigin - glm::normalize(stepFlatDelta) * stepRadius * 0.98f;
		glm::ivec3 hitIndex;
		glm::vec3 normal;
		World::currentWorld->rayCast(armTarget, castDirection, 5, radius, hitIndex, normal);
		interpolationFactor = 0.0f;
	}
	if(interpolationFactor >= 1.0f) 
	{
		armTargetInterpolated = armTarget;
	}
	else 
	{
		const glm::vec3 midPoint = (oldArmTarget + armTarget) * 0.5f - t.forward() * stepHeight;
		const glm::vec3 ab = glm::mix(oldArmTarget, midPoint, interpolationFactor);
		const glm::vec3 bc = glm::mix(midPoint, armTarget, interpolationFactor);
		armTargetInterpolated = glm::mix(ab, bc, interpolationFactor);
	}
	interpolationFactor += interpolationSpeed;
	invKinematics.setTarget(armTargetInterpolated);
	invKinematics.iterate(t.getPosition(), 2);
}

EXPORT_COMPONENT(GrabberArm);