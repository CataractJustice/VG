#include "DrillTool.h"
#include "GameNode/Properties/GameNodeProp.h"
#include "World.h"
#include "FollowNode.h"
#include "GameNode/ComponentFactory.h"
#include "Graphics/Shapes/Line3D.h"
#include "Components/InverseKinematics/Constraints.h"
DrillTool::DrillTool() 
{
	drillModelSurfaceDistance = 0.5f;
	range = 2.0f;
	drillModel = nullptr;
	drillIdlePosition = nullptr;
	rotationAcceleration = 0.01f;
	rotationMaxSpeed = 0.1f;
	rotationSpeed = 0.0f;
	addProp(new GameNodeProp((GameNode**)&drillModel), "Drill model");
	addProp(new GameNodeProp((GameNode**)&drillBitModel), "Drill bit model");
	addProp(new GameNodeProp((GameNode**)&drillIdlePosition), "Drill iddle");
	addProp(new GameNodeProp((GameNode**)&drillBaseHinge), "Drill base hinge");

	addProp(new GameNodeProp((GameNode**)&firstSmallHingeFirstPartAnchor), "First small hinge 1 anchor");
	addProp(new GameNodeProp((GameNode**)&firstSmallHingeSecondPartAnchor), "First small hinge 2 anchor");
	addProp(new GameNodeProp((GameNode**)&firstSmallHingeFirstPart), "First small hinge 1");
	addProp(new GameNodeProp((GameNode**)&firstSmallHingeSecondPart), "First small hinge 2");
	addProp(new GameNodeProp((GameNode**)&secondHingeAnchor), "Second hinge anchor");
	addProp(new GameNodeProp((GameNode**)&secondHinge), "Second hinge");

	addProp(new FloatProp(&drillBaseHingeOffset), "Drill base hinge offset");
	addProp(new FloatProp(&range), "Range");
	addProp(new FloatProp(&rotationAcceleration), "Rotation acceleration");
	addProp(new FloatProp(&rotationMaxSpeed), "Rotation max speed");
	addProp(new FloatProp(&drillModelSurfaceDistance), "Drill model surface distance");
	addProp(new FloatProp(&drillBaseHingeLength), "Drill base hinge length");
	
}

void DrillTool::onAttach() 
{
	NetworkItemBase::onAttach();
	GameNode* absolutePositioning = new GameNode();
	this->getParent()->addChild(absolutePositioning);
	drillActivePosition = new GameNode3D();
	absolutePositioning->addChild(drillActivePosition);
	followComp = new FollowNode();
	followComp->followNode = nullptr;
	followComp->positionFactor = 0.05f;
	followComp->rotationFactor = 0.05f;
	drillModel->attachComponent(followComp);
}

void DrillTool::updateItem() 
{
	if(!isUsageHeld(ItemUsageType::primary)) 
	{
		activeTicks = 0;
		followComp->followNode = drillIdlePosition;
		rotationSpeed *= 0.98f;
	}
	drillBitModel->getRelativeTransform().rotate(-rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 drillModelPosition = drillModel->getWorldTransform().getPosition();
	glm::vec3 itemPosition = this->getParentOfType<GameNode3D>()->getWorldTransform().getPosition();
	const glm::vec3 delta = itemPosition - drillModelPosition;
	const float distance = glm::length(delta);
	if(distance > 0.0f) 
	{
		const float maxDistance = (range + drillModelSurfaceDistance * 0.5f);
		const float minDistance = 0.3f;
		if(distance > maxDistance)
		{
			drillModel->getRelativeTransform().setPosition(drillModelPosition + (delta / distance * (distance - maxDistance)));
		}
		if(distance < minDistance)
		{
			drillModel->getRelativeTransform().translate(delta / distance * (distance - minDistance));
		}	
	}
	singleAxisHingeConstraint(drillModelPosition - drillModel->getWorldTransform().forward() * drillBaseHingeOffset, drillBaseHingeLength, drillModel->getWorldTransform().up(), -drillModel->getWorldTransform().forward(), 1.2f, drillBaseHinge);
	singleAxisHingeConstraint(
		firstSmallHingeFirstPartAnchor->getWorldTransform().getPosition(), 
		0.01f, firstSmallHingeFirstPartAnchor->getWorldTransform().up(), 
		firstSmallHingeFirstPartAnchor->getWorldTransform().forward(),
		1.1f,
		firstSmallHingeFirstPart
		);

	singleAxisHingeConstraint(
		firstSmallHingeSecondPartAnchor->getWorldTransform().getPosition(), 
		0.01f, firstSmallHingeSecondPartAnchor->getWorldTransform().up(), 
		firstSmallHingeSecondPartAnchor->getWorldTransform().forward(),
		10.0f,
		firstSmallHingeSecondPart
		);

	singleAxisHingeConstraint(
		secondHingeAnchor->getWorldTransform().getPosition(), 
		drillBaseHingeLength, 
		secondHingeAnchor->getWorldTransform().up(), 
		secondHingeAnchor->getWorldTransform().forward(), 
		10.0f, 
		secondHinge			
		);
	
}

void DrillTool::onUseHoldTick(unsigned char usage) 
{
	if(usage != ItemUsageType::primary) return;
	const Transform& t = this->getParentOfType<GameNode3D>()->getWorldTransform();
	glm::vec3 hitPosition = t.getPosition();
	glm::ivec3 hitIndex;
	glm::vec3 normal;
	bool hit = World::currentWorld->rayCast(hitPosition, t.forward(), 32, range, hitIndex, normal);
	followComp->followNode = hit ? drillActivePosition : drillIdlePosition;
	if(hit) 
	{
		Transform& dt = drillActivePosition->getRelativeTransform();
		dt.setPosition(hitPosition + normal * drillModelSurfaceDistance);
		float normalUpAlignment = glm::abs(glm::dot(t.up(), normal));
		dt.lookAt(-normal, normalUpAlignment < 0.99 ? t.up() : t.forward());
	}

	rotationSpeed += rotationAcceleration;
	rotationSpeed = rotationMaxSpeed > rotationSpeed ? rotationSpeed : rotationMaxSpeed;
	
	//glm::vec4 hitColor = glm::vec4(0.5f, 0.1f, 0.05f, 1.0f);
	//float lineWidth = 0.1f;
//
	//Core::shapesRenderer.pushShape(Line3D(hitPosition - glm::vec3(1.0f, 0.0f, 0.0f) * 0.1f, hitPosition + glm::vec3(1.0f, 0.0f, 0.0f) * 0.1f, hitColor * 1.8f, lineWidth));
	//Core::shapesRenderer.pushShape(Line3D(hitPosition - glm::vec3(0.0f, 1.0f, 0.0f) * 0.1f, hitPosition + glm::vec3(0.0f, 1.0f, 0.0f) * 0.1f, hitColor * 1.8f, lineWidth));
	//Core::shapesRenderer.pushShape(Line3D(hitPosition - glm::vec3(0.0f, 0.0f, 1.0f) * 0.1f, hitPosition + glm::vec3(0.0f, 0.0f, 1.0f) * 0.1f, hitColor * 1.8f, lineWidth));
	//Core::shapesRenderer.pushShape(Line3D(hitPosition, hitPosition + normal, glm::vec4(1.0f) - hitColor + glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), lineWidth));
	
}

EXPORT_COMPONENT(DrillTool);