#pragma once
#include "InventoryItemTemplates.h"
class InventorySlot;
struct InventoryItem 
{
	private:
	const InventoryItemTemplate* templatePtr;
	ItemTemplateId templateId;
	public:
	std::vector<std::shared_ptr<InventorySlot>> slots; 
	InventoryItem();
	InventoryItem(ItemTemplateId id, int amount);
	void init(ItemTemplateId id, int amount);
	int amount;
	const InventoryItemTemplate* getTemplatePtr() const;
	bool isEmpty() const;
	const ItemTemplateId& getId() const;
	void clear();
	std::vector<char> saveBuffer() const;
	void loadBuffer(const char* buffer);
	bool loadBufferSafe(const char* buffer, unsigned int size);
};