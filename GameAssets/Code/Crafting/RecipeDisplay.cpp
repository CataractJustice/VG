#include "RecipeDisplay.h"
#include "Recipes.h"
#include "Inventory/InventoryItemTemplates.h"
#include "Graphics/GUI/Background.h"
#include "Core.h"
#include "Graphics/GUI/MonospaceBitmapText.h"
#include "PlayerCrafting.h"
RecipeDisplay::RecipeDisplay(const Recipe* recipe, unsigned int recipeIndex, PlayerCrafting* crafting) : 
recipe(recipe),
playerCrafting(crafting),
recipeIndex(recipeIndex)
{
	
}

unsigned int craftingIconTextureId;
bool craftingIconTextureIdFound = false;

class TryCraftOnClick : public Component 
{
	private:
	unsigned int index;
	PlayerCrafting* crafting;

	public:
	TryCraftOnClick(unsigned int index, PlayerCrafting* crafting) : index(index), crafting(crafting)
	{

	}
	void onClick(unsigned short bttn) override 
	{
		crafting->tryCraftByRecipeIndex(index);
	}
};

GameNodeGUI* newIcon(unsigned int textureImage, unsigned int count, const GUIUnit& iconUnit, GUIUnit& iconOffset) 
{
	GameNodeGUI* icon = new GameNodeGUI();
	Background* iconBg = new Background();
	iconBg->setColor(glm::vec4(0.0f));
	iconBg->setPressColor(glm::vec4(0.0f));
	if(count) 
	{
		const GUIUnit textSize = GUIUnit::pixels(11);
		MonospaceBitmapText* text = new MonospaceBitmapText();
		text->setTexture(&Core::textureManager.get("11x11font.png"));
		text->setColor(glm::vec4(1.0f));
		const std::string countStr = std::to_string(count);
		text->setText(countStr);
		text->setSize(textSize);
		text->setLeft(iconUnit - (textSize * countStr.size()));
		text->setTop(iconUnit - textSize);
		icon->attachComponent(text);
	}
	icon->getTransform().setWidth(iconUnit);
	icon->getTransform().setHeight(iconUnit);
	icon->getTransform().setOffsetX(iconOffset);
	iconBg->setTexture(textureImage);
	icon->attachComponent(iconBg);
	iconOffset = iconOffset + iconUnit;
	return icon;
}

void RecipeDisplay::onAttach() 
{
	if(craftingIconTextureIdFound == false) 
	{
		craftingIconTextureIdFound = true;
		craftingIconTextureId = Core::textureManager.get("PlayerCraftingCraftIcon.png").id();
	}
	GameNodeGUI* gui = this->getParentOfType<GameNodeGUI>();
	if(!gui) throw std::runtime_error("RecipesDisplay must be attached to a gui node");
	GUIUnit iconUnit = GUIUnit::parentHeights(1.0f);
	GUIUnit iconOffset = GUIUnit::parentHeights(0.0f);
	for(auto[id, count] : recipe->getRequiredItems()) 
	{
		const InventoryItemTemplate* itemTemplate = InventoryItemTemplates::getInstance().getById(id);
		GameNodeGUI* itemIcon = newIcon(itemTemplate->textureImage, count, iconUnit, iconOffset);
		this->getParent()->addChild(itemIcon);
	}
	
	GameNodeGUI* craftIcon = newIcon(craftingIconTextureId, 0, iconUnit, iconOffset);
	TryCraftOnClick* clickCraft = new TryCraftOnClick(recipeIndex, playerCrafting);
	craftIcon->attachComponent(clickCraft);
	this->getParent()->addChild(craftIcon);

	for(auto[id, count] : recipe->getOutputItems()) 
	{
		const InventoryItemTemplate* itemTemplate = InventoryItemTemplates::getInstance().getById(id);
		GameNodeGUI* itemIcon = newIcon(itemTemplate->textureImage, count, iconUnit, iconOffset);
		this->getParent()->addChild(itemIcon);
	}
}