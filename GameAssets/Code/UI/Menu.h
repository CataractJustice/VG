#include "World.h"
using namespace TealEngine;

class Menu
{
	public:
	static void newWorld(const std::string& name, int seed);
	static void loadWorld(const std::string& name);
};