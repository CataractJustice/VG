#include "SunController.h"
#include "GameNode/ComponentFactory.h"
#include "GameNode/Properties/GameNodeProp.h"
SunController::SunController() : player(nullptr), dl(nullptr)
{
	addProp(new GameNodeProp(&player), "Player");
}

void SunController::update() 
{
	if(this->player == nullptr || GameNode::isValidNode(player) == false) return;
	if(this->dl == nullptr || Component::isValidComponent(dl) == false) 
	{
		auto dls = this->getParent()->findComponentsByType<DirectionLight>();
		if(dls.size() <= 0) return;
		dl = dls[0];
	}
	
}

EXPORT_COMPONENT(SunController);
