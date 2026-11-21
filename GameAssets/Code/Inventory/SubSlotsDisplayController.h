#include "InventorySlotComponent.h"
#include "GameNode/GameNodeGUI.h"
class SubSlotsDisplayController : public Component
{
	private:
	InventorySlotComponent* slotCompPtr = nullptr;
	GameNodeGUI* gui = nullptr;
	std::vector<GameNodeGUI*> slots;
	InventorySlot* lastSlot = nullptr;
	public:
	inline void setObservedSlot(InventorySlotComponent* slotComp) { this->slotCompPtr = slotComp; };
	void update() override;
	void onAttach() override;
	void refreshSlots();
};	