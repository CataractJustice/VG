#pragma once
#include "GameNode/Component.h"
#include "Inventory/InventorySlotComponent.h"
#include <map>

using namespace TealEngine;

class ProcessMaterial : public Component 
{
	private:
	struct Recipe
	{
		int inAmount;
		int outAmount;
		int ticks;
		std::string outItemId;
	};

	std::map<std::string, Recipe> recipes;
	std::map<std::string, Recipe>::iterator currentRecipe;
	InventorySlotComponent* inputSlot;
	InventorySlotComponent* outputSlot;
    bool crafting;
    int craftingTicks;
	bool canProcess();
	public:
	ProcessMaterial();
	void update() override;
	void onClick(unsigned short button) override;
};