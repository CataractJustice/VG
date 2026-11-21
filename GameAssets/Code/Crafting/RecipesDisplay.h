#pragma once
#include "Recipes.h"
#include "GameNode/Component.h"
using namespace TealEngine;
class Inventory;
class PlayerBuilder;
namespace TealEngine 
{
	class GameNodeGUI;
}
class RecipesDisplay : public Component 
{
	private:
	const Inventory* inventory;
	const Recipes* recipes;
	PlayerBuilder* player;
	unsigned int updateCounter;
	std::vector<std::pair<GameNodeGUI*, unsigned int>> lastAvailableRecipes;
	public:
	RecipesDisplay(const Recipes* recipes, const Inventory* inventory, PlayerBuilder* player);
	void update() override;
};