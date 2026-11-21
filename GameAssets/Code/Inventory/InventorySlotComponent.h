#pragma once
#include "InventorySlot.h"
#include "GameNode/Component.h"
using namespace TealEngine;
namespace TealEngine 
{
	class Background;
	class MonospaceBitmapText;
}

class InventorySlotComponent : public Component 
{
	Background* bgc;
	MonospaceBitmapText* text;
	InventorySlot* slotPtr;
	bool picturePresent;
	unsigned int currentPicture;
	bool showSubSlots;
	public:
	static InventoryItem heldItem;
	inline InventorySlot& getSlot() { return *slotPtr; };
	inline InventorySlot* getSlotPtr() { return slotPtr; };
	void setSlot(InventorySlot* slot = nullptr);
	void onClick(unsigned short button) override;
	void onAttach() override;
	void update() override;
	InventorySlotComponent();
	~InventorySlotComponent();
};