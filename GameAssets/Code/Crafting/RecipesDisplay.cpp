#include "RecipesDisplay.h"
#include "Recipes.h"
#include "Inventory/Inventory.h"
#include "GameNode/GameNodeGUI.h"
#include "RecipeDisplay.h"
#include "Player/PlayerBuilder.h"
//This is also a material display width
const GUIUnit displayEntryHeight = GUIUnit::pixels(64);
const GUIUnit displayEntryPadding = GUIUnit::pixels(4);

RecipesDisplay::RecipesDisplay(const Recipes* recipes, const Inventory* inventory, PlayerBuilder* player) :
recipes(recipes),
inventory(inventory),
player(player)
{

}

void RecipesDisplay::update() 
{
	updateCounter++;
	if(updateCounter % 32) return;
	unsigned int availableRecipesStackIndex = 0;
	unsigned int recipeIndex = -1;
	for(const Recipe& recipe : recipes->getRecipes()) 
	{
		recipeIndex++;
		if(!recipe.checkRequirements(inventory->getItemCountsMap())) continue;
		if(lastAvailableRecipes.size() <= availableRecipesStackIndex) 
		{
			/*
			Add new displayed recipe
			*/
			GameNodeGUI* recipeContainer = new GameNodeGUI();
			recipeContainer->getTransform().setOffsetY((displayEntryHeight + displayEntryPadding) * availableRecipesStackIndex);
			recipeContainer->getTransform().setWidth(GUIUnit::parentWidths(1.0f));
			recipeContainer->getTransform().setHeight(displayEntryHeight);
			lastAvailableRecipes.push_back({recipeContainer, recipeIndex});
			recipeContainer->attachComponent(new RecipeDisplay(&recipe, recipeIndex, player->crafting));
			this->getParent()->addChild(recipeContainer);
		}
		else 
		{
			if(lastAvailableRecipes[availableRecipesStackIndex].second != recipeIndex) 
			{
				/*
				Change displayed recipe
				*/
				lastAvailableRecipes[availableRecipesStackIndex].first->destroy();
				GameNodeGUI* recipeContainer = new GameNodeGUI();
				recipeContainer->getTransform().setOffsetY((displayEntryHeight + displayEntryPadding) * availableRecipesStackIndex);
				recipeContainer->getTransform().setWidth(GUIUnit::parentWidths(1.0f));
				recipeContainer->getTransform().setHeight(displayEntryHeight);
				recipeContainer->attachComponent(new RecipeDisplay(&recipe, recipeIndex, player->crafting));
				this->getParent()->addChild(recipeContainer);
				lastAvailableRecipes[availableRecipesStackIndex].first = recipeContainer;
				lastAvailableRecipes[availableRecipesStackIndex].second = recipeIndex;
			}
		}
		availableRecipesStackIndex++;
	}
	while(lastAvailableRecipes.size() > availableRecipesStackIndex) 
	{
		lastAvailableRecipes.back().first->destroy();
		lastAvailableRecipes.pop_back();
	}
}