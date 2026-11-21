#include "Menu.h"
#include "World.h"
#include "Player/PlayerSpawner.h"
#include <fstream>
#include "System/Logger.h"
#include "System/Random.h"
void Menu::newWorld(const std::string& name, int seed) 
{
	//Create New world node
	//Load some chunks with chunk loader
	//Put player somewhere on top
	World::worldData.seed = seed;
	//copy title
	memcpy(World::worldData.title, name.c_str(), std::min(sizeof(World::worldData.title) / sizeof(char) - 1, name.length()));
	//generate random id
	randomBytes(World::worldData.id, worldIdSize);
	Core::setScene("./GameAssets/World.node");
}

void Menu::loadWorld(const std::string& name) 
{
	memcpy(World::worldData.title, name.c_str(), std::min(sizeof(World::worldData.title) / sizeof(char) - 1, name.length()));
	Core::setScene("./GameAssets/World.node");
}