#include "GameNode/Component.h"

using namespace TealEngine;

class FollowNode : public Component
{
	private:
	public:
	float rotationFactor;
	float positionFactor;
	GameNode* followNode;
	FollowNode();
	void update() override;
};