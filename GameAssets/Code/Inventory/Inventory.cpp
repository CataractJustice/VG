#include "Inventory.h"
#include "Core.h"
#include "GameNode/ComponentFactory.h"
#include "System/Input.h"
#include "GameNode/GameNodeGUI.h"
#include "GameNode/Properties/GameNodeProp.h"

Inventory::Inventory()
{
}


int Inventory::pushStack(InventoryItem item) 
{
	int itemsAdded = 0;
	for(const auto& slot : slots) 
	{
		int delta = slot->pushStack(item);
		item.amount -= delta;
		itemsAdded += delta;
		if(item.amount <= 0) return itemsAdded;
	}
	return itemsAdded;
}

int Inventory::pushStackCheck(InventoryItem item) 
{
	int itemsAdded = 0;
	for(const auto& slot : slots) 
	{
		int delta = slot->pushStackCheck(item);
		item.amount -= delta;
		itemsAdded += delta;
		if(item.amount <= 0) return itemsAdded;
	}
	return itemsAdded;
}

void Inventory::addSlot(InventorySlot* slot) 
{
	slot->setInventory(this);
	this->slots.push_back(slot);
}

//I had a dream where I strangle you with both of my hands
std::vector<char> Inventory::saveBuffer() 
{
	std::vector<char> buffer;
	int slotCount = slots.size();
	buffer.insert(buffer.end(), (char*)&slotCount, (char*)&slotCount + sizeof(int));
	for(InventorySlot* slot : slots) 
	{
		std::vector<char> slotBuffer = slot->saveBuffer();
		buffer.insert(buffer.end(), slotBuffer.begin(), slotBuffer.end());
	}
	return buffer;
}

//Slot format is [int count][string id][\0]
void Inventory::loadBuffer(const char* buffer) 
{
	int slotsCount = *(int*)buffer;
	buffer += sizeof(int);
	for(InventorySlot* slot : slots) 
	{
		if(slotsCount == 0) return;
		slot->loadBuffer(buffer);
		buffer += sizeof(int);
		while(*(buffer++));
		slotsCount--;
	}
}

bool Inventory::loadBufferSafe(const char* buffer, unsigned int size) 
{
	if(size <= sizeof(int)) return false;
	int slotsCount = *(const int*)buffer;
	if(slotsCount <= 0 || slotsCount > 3200) return false;
	size -= sizeof(int);
	buffer += sizeof(int);
	if(size <= slotsCount * sizeof(int)) return false;
	for(InventorySlot* slot : slots) 
	{
		if(slotsCount == 0) return false;
		if(!slot->loadBufferSafe(buffer, size)) return false;
		buffer += sizeof(int);
		size -= sizeof(int);
		//guaranteed by slot->loadBufferSafe to be null terminated within size.
		//reach null terminator
		while(*buffer) 
		{
			buffer++;
			size--;
		}
		//move to next slot
		buffer++;
		size--;
		slotsCount--;
	}
	return true;
}
std::map<std::string, unsigned int> Inventory::getItemCountsMap() const 
{
	std::map<std::string, unsigned int> itemCounts;
	for(const InventorySlot* slot : slots) 
	{
		if(slot->getItemConst().isEmpty()) continue;
		itemCounts[slot->getItemConst().getId()] += slot->getItemConst().amount;
	}
	return itemCounts;
}

bool Inventory::removeItemsByIdAndCount(std::map<std::string, unsigned int> itemsToRemove) 
{
	for(InventorySlot* slot : slots) 
	{
		auto it = itemsToRemove.find(slot->getItem().getId());
		if(it == itemsToRemove.cend()) continue;
		const unsigned int delta = std::min(it->second, (unsigned int)slot->getItem().amount);
		slot->getItem().amount -= delta;
		it->second -= delta;
		slot->sync();
	}
	for(auto[id, count] : itemsToRemove) 
	{
		if(count) return false;
	}
	return true;
}

// Returns false if some items could not fit, writes items that could not fit into second argument
bool Inventory::addItemsByIdAndCount(std::map<std::string, unsigned int> itemsToAdd, std::map<std::string, unsigned int>& itemsThatCouldNotFit)
{
	if(itemsToAdd.empty()) throw std::invalid_argument("Empty itemsToAdd passed to addItemsByIdAndCount, why?");
	for (InventorySlot* slot : slots)
	{
		if(slot->getItem().isEmpty()) 
		{
			auto&[id, count] = *itemsToAdd.begin();
			unsigned int delta = std::min(count, (unsigned int)InventoryItemTemplates::getInstance().getById(id)->maxStack);
			slot->getItem().init(id, delta);
			count -= delta;
			slot->sync();
			continue;
		}
		auto it = itemsToAdd.find(slot->getItem().getId());
		if (it == itemsToAdd.end()) continue;

		const unsigned int available = slot->getItem().getTemplatePtr()->maxStack - slot->getItem().amount;
		const unsigned int delta = std::min(it->second, available);

		slot->getItem().amount += delta;
		it->second -= delta;
		slot->sync();
	}

	for (const auto& [id, count] : itemsToAdd)
	{
		if (count)
			itemsThatCouldNotFit[id] = count;
	}

	return itemsThatCouldNotFit.empty();
}