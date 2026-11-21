#pragma once
#include "GameNode/Component.h"
#include "Inventory/InventoryItem.h"
using namespace TealEngine;

class EquippedItem : public Component 
{
	private:

	public:
	InventoryItem* itemPtr;
	InventorySlot* slotPtr;
	bool mainPlayer;
	unsigned int slotIndex;
	unsigned int playerId;
	inline EquippedItem() : itemPtr(nullptr) {  }
	inline EquippedItem(InventoryItem* itemPtr, InventorySlot* slotPtr, bool mainPlayer = true, unsigned int slotIndex = 0, unsigned int playerId = 0) : 
	slotIndex(slotIndex), 
	itemPtr(itemPtr), 
	slotPtr(slotPtr),
	mainPlayer(mainPlayer), 
	playerId(playerId) {  }
};	