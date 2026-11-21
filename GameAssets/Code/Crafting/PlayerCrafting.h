#pragma once
#include "GameNode/Component.h"

using namespace TealEngine;
class Recipes;
class Inventory;
class PlayerBuilder;
class PlayerCrafting : public Component 
{
	private:
	const Recipes* recipes;
	Inventory* inventory;
	const PlayerBuilder* player;
	public:
	PlayerCrafting(const Recipes* recipes, Inventory* playerInventory, const PlayerBuilder* player);
	void tryCraftByRecipeIndex(unsigned int recipeIndex);
};