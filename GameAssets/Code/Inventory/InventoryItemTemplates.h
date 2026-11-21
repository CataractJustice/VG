#pragma once
#include <string>
#include <map>
#include <functional>
#include "libs/glm/glm.hpp"
#include "NlohmannJson/json.hpp"
#include <unordered_set>

using Json = nlohmann::json;

typedef std::string ItemTemplateId;
struct SlotTemplate 
{
	bool useWhiteList;
	std::unordered_set<ItemTemplateId> whiteList;
	std::unordered_set<ItemTemplateId> blackList;	
};
class InventoryItemTemplate
{
	private: 
	public:
	ItemTemplateId id;
	unsigned int textureImage;
	std::string name;
	std::string onEquipNodePath;
	std::string onOtherEquipNodePath;
	Json defaultData;
	int maxStack;
	std::vector<SlotTemplate> slots;
	InventoryItemTemplate();
};

class InventoryItemTemplates 
{
	private:
	std::map<ItemTemplateId, InventoryItemTemplate> itemTemplatesMap;
	static InventoryItemTemplates* instance;

	public:
	InventoryItemTemplates();

	const InventoryItemTemplate* getById(ItemTemplateId id) const; 
	InventoryItemTemplate& addItemTemplate(ItemTemplateId id, InventoryItemTemplate itemTemplate);
	static InventoryItemTemplates& getInstance();

};