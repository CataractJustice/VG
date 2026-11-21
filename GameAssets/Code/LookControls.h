#pragma once
#include "GameNode/Component.h"
class LookControls : public TealEngine::Component 
{
	private:
	glm::vec2 view;
	public:
	glm::vec2 lookSensitivity;
	void update() override;
	LookControls();
};