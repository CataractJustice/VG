#pragma once
#include "InventoryItem.h"
#include "InventorySlot.h"

class Inventory 
{
	private:
	std::vector<InventorySlot*> slots;
	public:
	Inventory();
	int pushStack(InventoryItem item);
	int pushStackCheck(InventoryItem item);
	void addSlot(InventorySlot* slot);
	std::vector<char> saveBuffer();
	void loadBuffer(const char* buffer);
	bool loadBufferSafe(const char* buffer, unsigned int size);
	std::map<std::string, unsigned int> getItemCountsMap() const;
	//returns true if enough items was removed, false if there was not enough items, 
	//removes items either way up to specified count
	bool removeItemsByIdAndCount(std::map<std::string, unsigned int> itemsToRemove);
	bool addItemsByIdAndCount(std::map<std::string, unsigned int> itemsToAdd, std::map<std::string, unsigned int>& itemsThatCouldNotFit);
};