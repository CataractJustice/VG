#include "Core.h"
#include "NodeTrail.h"
#include "GameNode/ComponentFactory.h"
NodeTrail::NodeTrail() 
{
	this->addProp(new IntProp(&segmentCount), "Segment count");
	this->addProp(new FloatProp(&distance), "Segment distance");
	this->addProp(new StringProp(&nodePath), "Segment node path");
	segmentCount = 64;
	distance = 1.8f;
	this->step = 0.05f;
}

void NodeTrail::onAttach() 
{
	for(int i = 0; i < this->segmentCount; i++) 
	{
		GameNode3D* segmentNode = (GameNode3D*)GameNode::loadNodeFromJsonFile(nodePath);
		this->segments.push_back(segmentNode);
		Core::getRoot()->addChild(segmentNode);
	}
}

void NodeTrail::update() 
{
	const Transform& transform = this->getParentOfType<GameNode3D>()->getWorldTransform();
	float oldDist = glm::distance(this->oldPos, transform.getPosition());
	float maxLag = segments.size() * distance;
	if(oldDist > step) 
	{
		//add trail to trail queue
		this->trail.push_back({oldPos, oldQuat, 0.0f});
		for(Trail& t : trail) 
		{
			t.lag += oldDist;
		}
		oldPos = transform.getPosition();
		oldQuat = transform.getRotation();
	}
	while (trail.size() && trail[0].lag > maxLag)
	{
		trail.erase(trail.begin());
	}
	
	int trailInd = this->trail.size() - 1;
	for(int i = 0; i < this->segments.size(); i++) 
	{
		const float lag = i * this->distance;
		while(trailInd >= 2 && this->trail[trailInd-1].lag < lag) 
		{
			trailInd--;
		}
		if(trailInd == 0) break;
		if(this->trail.size() < 2) break;
		Trail& from = this->trail[trailInd];
		Trail& to = this->trail[trailInd - 1];
		float trailDist = from.lag - to.lag;
		float lagDist = from.lag - lag;
		segments[i]->getRelativeTransform().setPosition(glm::mix(from.position, to.position, lagDist / trailDist));
		segments[i]->getRelativeTransform().setRotation(glm::slerp(from.orientation, to.orientation, lagDist / trailDist));
	}
}

EXPORT_COMPONENT(NodeTrail);
