#pragma once
#include "GameNode/GameNode.h"
#include "GameNode/Component.h"
#include "Inventory/InventorySlotComponent.h"

using namespace TealEngine;

class ItemEquipper : public Component
{
	private:
	GameNode* currentItemObject;
	GameNode* equipNode;
	std::vector<InventorySlotComponent*> slotDisplays;
	std::vector<InventorySlot*> slots;
	unsigned int selectedItemIndex;
	public:
	bool mainPlayer;
	unsigned int playerId;
	void addSlot(InventorySlot* slot, InventorySlotComponent* display = nullptr);
	ItemEquipper();
	unsigned int getSlotIndex() { return selectedItemIndex; };
	void updateItemObject();
	void selectItemSlot(int index);
	void update() override;
	inline GameNode* getItemObject() { return currentItemObject; }
	InventoryItem& getEquipedItemStack();
	inline void setEquipNode(GameNode* node) { equipNode = node; }
};
