#include "MenuOpenScene.h"
#include "Core.h"
#include "GameNode/ComponentFactory.h"
MenuOpenScene::MenuOpenScene()
{
	this->addProp(new StringProp(&this->scenePath), "Scene path");
}

void MenuOpenScene::onClick(unsigned short button) 
{
	Core::setScene(this->scenePath);
}

EXPORT_COMPONENT(MenuOpenScene);