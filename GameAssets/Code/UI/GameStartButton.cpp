#include "GameStartButton.h"
#include "Core.h"
#include "GameNode/Properties/ComponentRefProp.h"
#include "GameNode/ComponentFactory.h"
#include "Menu.h"
#include "Math/Noise.h"
#include "../Menu/MenuInit.h"
GameStartButton::GameStartButton() 
{
	worldName = "";
}

void GameStartButton::setWorldName(const std::string& name) 
{
	this->worldName = name;
}

void GameStartButton::onClick(unsigned short button) 
{

	if(this->worldName.length()) 
	{
		//load world
		Menu::newWorld(this->worldName, 0);
		return;
	}
	std::string seedStr = MenuInit::getMenuInstance()->getInputValue("seed");

	unsigned int seed = 0;
	int power = 0;
	for(char c : seedStr) 
	{
		seed += Noise::uintGet(seed ^ ((unsigned int)(c) << ((power % 4) * 8)));
		power++;
	}
	Menu::newWorld(MenuInit::getMenuInstance()->getInputValue("name"), seed);
}

EXPORT_COMPONENT(GameStartButton);	