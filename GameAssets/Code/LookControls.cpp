#include "LookControls.h"
#include "GameNode/ComponentProp.h"
#include "GameNode/ComponentFactory.h"
#include "System/Input.h"
#include "GameNode/GameNode3D.h"
#include "libs/glm/gtc/type_ptr.hpp"

using namespace TealEngine;

LookControls::LookControls() 
{
	lookSensitivity = glm::vec2(0.01f);
	view = glm::vec2(0.0f);
	this->addProp(new FloatVecProp(glm::value_ptr(lookSensitivity), 2), "Look sensitivity");
	this->addProp(new FloatVecProp(glm::value_ptr(view), 2), "View");
}

void LookControls::update() 
{
	Transform& transform = ((GameNode3D*)this->getParent())->getRelativeTransform();
	glm::vec2 deltaMouse = Input::Mouse::getDeltaMousePos();
	this->view += deltaMouse * this->lookSensitivity * glm::vec2(-1.0f, 1.0f);
	transform.setRotation(view.x, glm::vec3(0.0f, 1.0f, 0.0f));
	transform.rotate(view.y, glm::vec3(1.0f, 0.0f, 0.0f));
}

EXPORT_COMPONENT(LookControls);