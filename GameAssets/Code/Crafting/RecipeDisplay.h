#pragma once
#include "Graphics/GUI/GUIComponent.h"
using namespace TealEngine;
class Recipe;
class PlayerCrafting;
class RecipeDisplay : public Component 
{
	private:
	const Recipe* recipe;
	PlayerCrafting* playerCrafting;
	unsigned int recipeIndex;
	public:
	RecipeDisplay(const Recipe* recipe, unsigned int recipeIndex, PlayerCrafting* crafting);
	void onAttach() override;
};