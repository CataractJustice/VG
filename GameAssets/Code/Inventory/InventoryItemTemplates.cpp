#include "InventoryItemTemplates.h"
#include "VoxelMaterials.h"
#include "Core.h"
#include "Graphics/Renderer/RenderUtil.h"
#include <stdexcept>
#include "Core.h"

using namespace TealEngine;

InventoryItemTemplates* InventoryItemTemplates::instance = nullptr;

InventoryItemTemplate::InventoryItemTemplate() :
onEquipNodePath(""),
onOtherEquipNodePath(""),
maxStack(1),
name(""),
textureImage(0),
defaultData({}),
slots({})
{

}

InventoryItemTemplates::InventoryItemTemplates() 
{
	auto& vmatsmap = VoxelMaterials::getInstance().getMaterials();
	const int materialItemMaxStack = 1200;
	for(int id = 0; id < vmatsmap.size(); id++) 
	{
		const VoxelMaterial& material = vmatsmap[id];
		InventoryItemTemplate itemTemplate;
		itemTemplate.onEquipNodePath = "./GameAssets/Items/MaterialItem.node";
		itemTemplate.maxStack = materialItemMaxStack;
		itemTemplate.name = material.name;
		itemTemplate.textureImage = material.itemTexture;//Core::textureManager.get("Item." + material.name + ".png").id();
		itemTemplate.defaultData = {};
		itemTemplate.slots = {};
		itemTemplate.defaultData["id"] = id;

		this->addItemTemplate("material."+material.name, itemTemplate);
	}
	SlotTemplate energySlotTemplate;
	energySlotTemplate.useWhiteList = true;
	energySlotTemplate.whiteList = {"material.mana_crystal"};

	InventoryItemTemplate itemTemplate;
	itemTemplate.maxStack = 1;
	itemTemplate.name = "Laser";
	itemTemplate.textureImage = Core::textureManager.get("MainToolItemPic.png").id();
	itemTemplate.onEquipNodePath = "./GameAssets/Items/MiningToolItem.node";
	itemTemplate.defaultData = {};
	itemTemplate.slots = {energySlotTemplate};
	this->addItemTemplate("mining_tool", itemTemplate);

	itemTemplate.maxStack = 1;
	itemTemplate.name = "Staff";
	itemTemplate.textureImage = Core::textureManager.get("MainToolItemPic.png").id();
	itemTemplate.onEquipNodePath = "./GameAssets/Items/Staff.node";
	itemTemplate.defaultData = {};
	itemTemplate.slots = {energySlotTemplate};
	this->addItemTemplate("staff", itemTemplate);

	itemTemplate;
	itemTemplate.maxStack = 1;
	itemTemplate.name = "MassFab";
	itemTemplate.textureImage = Core::textureManager.get("hisui.png").id();
	itemTemplate.onEquipNodePath = "./GameAssets/Items/BHoleGun.node";
	itemTemplate.defaultData = {};
	itemTemplate.slots = {};
	this->addItemTemplate("bhole_gun", itemTemplate);

	itemTemplate.maxStack = 1;
	itemTemplate.name = "One (1) Gun";
	itemTemplate.textureImage =  Core::textureManager.get("OneGunItem.png").id();
	itemTemplate.onEquipNodePath = "./GameAssets/Items/Gun.node";
	itemTemplate.defaultData = {};
	itemTemplate.slots = {};
	this->addItemTemplate("one_gun", itemTemplate);

	itemTemplate.maxStack = 1;
	itemTemplate.name = "Drill";
	itemTemplate.textureImage =  Core::textureManager.get("MainToolItemPic.png").id();
	itemTemplate.onEquipNodePath = "./GameAssets/Items/DrillItem.node";
	itemTemplate.defaultData = {};
	itemTemplate.slots = {};
	this->addItemTemplate("drill", itemTemplate);
}

const InventoryItemTemplate* InventoryItemTemplates::getById(ItemTemplateId id) const
{
	if(itemTemplatesMap.find(id) == itemTemplatesMap.cend()) return nullptr;
	return &(this->itemTemplatesMap.find(id)->second);
}

InventoryItemTemplate& InventoryItemTemplates::addItemTemplate(ItemTemplateId id, InventoryItemTemplate itemTemplate) 
{
	if(itemTemplate.slots.size()) 
	{
		if(itemTemplate.maxStack != 1) 
		{
			throw std::invalid_argument("inventory item template with slots must not stack");
		}
	}
	this->itemTemplatesMap[id] = itemTemplate;
	return this->itemTemplatesMap[id];
}

InventoryItemTemplates& InventoryItemTemplates::getInstance() 
{
	if(instance == nullptr) 
	{
		instance = new InventoryItemTemplates();
	}
	return *instance;
}
