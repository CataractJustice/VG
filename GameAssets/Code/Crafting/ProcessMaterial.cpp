#include "ProcessMaterial.h"
#include "GameNode/Properties/ComponentRefProp.h"
#include "GameNode/ComponentFactory.h"

bool ProcessMaterial::canProcess() 
{
	const auto& inputItem	= inputSlot->getSlot().getItem();
	const auto& outputItem	= outputSlot->getSlot().getItem();
	const auto& recipe		= currentRecipe->second;
	if(!inputSlot) return false;
	if(currentRecipe == recipes.cend()) return false;
	if(inputItem.amount < recipe.inAmount) return false;
	if(inputItem.getId() != currentRecipe->first) return false;
	if(outputItem.amount != 0) 
	{
		if(outputItem.getId() != recipe.outItemId) return false;
		if((outputItem.amount + recipe.outAmount) > outputItem.getTemplatePtr()->maxStack) return false; 
	}
	return true;
}

ProcessMaterial::ProcessMaterial() : inputSlot(0), outputSlot(0)
{
	this->addProp(new ComponentRefProp<InventorySlotComponent>(&inputSlot), "Input slot");
	this->addProp(new ComponentRefProp<InventorySlotComponent>(&outputSlot), "Output slot");
	crafting = 0;
	craftingTicks = 0;
	currentRecipe = recipes.end();
	this->recipes["material.slate"] = 
	{
		1, 1,
		60,
		"material.tiles"
	};
}
void ProcessMaterial::update() 
{
	if(!inputSlot || !outputSlot) return;
	if (crafting) 
	{
		if(!canProcess()) 
		{
			crafting = false;
			return;
		}
		if(craftingTicks >= currentRecipe->second.ticks) 
		{
			this->inputSlot->getSlot().getItem().amount -= currentRecipe->second.inAmount;
			if(this->outputSlot->getSlot().getItem().amount == 0) 
			{
				this->outputSlot->getSlot().pushStack(InventoryItem(currentRecipe->second.outItemId, currentRecipe->second.outAmount));
			}
			else 
			{
				this->outputSlot->getSlot().getItem().amount += currentRecipe->second.outAmount;
			}
			craftingTicks = 0;
		}
		craftingTicks++;
	}
}

void ProcessMaterial::onClick(unsigned short button) 
{
	if(!inputSlot || !outputSlot) return;
	if (crafting) {
		crafting = false;
		craftingTicks = 0;
	} else {
		std::string itemId = inputSlot->getSlot().getItem().getId();
		auto it = recipes.find(itemId);
		if (it != recipes.end()) {
			if (inputSlot->getSlot().getItem().amount >= it->second.inAmount) {
				crafting = true;
				currentRecipe = it;
			}
		}
	}
}

EXPORT_COMPONENT(ProcessMaterial);