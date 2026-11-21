#include "SubSlotsDisplayController.h"

void SubSlotsDisplayController::update() 
{
	if(!slotCompPtr) return;
	if(!gui) return;

	int subSlotCount = slotCompPtr->getSlot().getItem().getTemplatePtr() ? 
	slotCompPtr->getSlot().getItem().getTemplatePtr()->slots.size() : 0;
	GUITransform& t = slotCompPtr->getParentOfType<GameNodeGUI>()->getTransform();
	GUIUnit top = t.getTop();
	GUIUnit left = t.getLeft();
	GUIUnit width = t.getWidth();
	this->gui->getTransform().setOffsetX(left);
	this->gui->getTransform().setWidth(GUIUnit::pixels(width.inPixels(t)));
	this->gui->getTransform().setHeight(GUIUnit::pixels(width.inPixels(t)) * subSlotCount);
	this->gui->getTransform().setOffsetY(top - this->gui->getTransform().getHeight());
	InventorySlot* newSlot = nullptr;
	if(this->slotCompPtr->getSlot().getItem().slots.size()) 
	{
		newSlot = this->slotCompPtr->getSlot().getItem().slots[0].get();
	}
	if(newSlot != lastSlot) 
	{
		refreshSlots();
		lastSlot = newSlot;
	}
}
void SubSlotsDisplayController::onAttach()
{
	gui = this->getParentOfType<GameNodeGUI>();
}
void SubSlotsDisplayController::refreshSlots() 
{
	for(GameNodeGUI* slotNode : this->slots) 
	{
		slotNode->destroy();
	}
	slots.clear();
	auto& itemSlots = this->slotCompPtr->getSlot().getItem().slots;
	int slotIndex = 0;
	for(auto& slot : itemSlots) 
	{
		GameNodeGUI* slotNode = new GameNodeGUI();
		slotNode->getTransform().setWidth(GUIUnit::parentWidths(1.0f));
		slotNode->getTransform().setHeight(GUIUnit::parentWidths(1.0f));
		slotNode->getTransform().setOffsetY(GUIUnit::parentHeights(1.0f) - GUIUnit::parentWidths(float(slotIndex) + 1.0f));
		InventorySlotComponent* isc = new InventorySlotComponent();
		gui->addChild(slotNode);
		isc->setSlot(slot.get());
		slotNode->attachComponent(isc);
		slots.push_back(slotNode);
		slotIndex++;
	}
}