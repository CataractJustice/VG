#include "FollowNode.h"
#include "GameNode/GameNode3D.h"
#include "GameNode/ComponentFactory.h"
#include "GameNode/Properties/GameNodeProp.h"
void FollowNode::update() 
{
	if(followNode == nullptr) return;
	if(GameNode::isValidNode(followNode) == false) return;
	GameNode3D* parent3d = (GameNode3D*)getParent();
	GameNode3D* follow3d = (GameNode3D*)followNode;

	Transform& st = parent3d->getRelativeTransform();
	const Transform& dt = follow3d->getWorldTransform();
	st.setPosition(st.getPosition() * (1.0f - positionFactor) + dt.getPosition() * positionFactor);
	st.setRotation(glm::slerp(st.getRotation(), dt.getRotation(), rotationFactor));
}

FollowNode::FollowNode() 
{
	followNode = nullptr;
	rotationFactor = 0.5f;
	positionFactor = 0.5f;
	this->addProp(new FloatProp(&rotationFactor), "Rotation follow factor");
	this->addProp(new FloatProp(&positionFactor), "Position follow factor");
	this->addProp(new GameNodeProp(&followNode), "Follow node");
}

EXPORT_COMPONENT(FollowNode);