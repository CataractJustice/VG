#include "NetworkItemBase.h"
#include "Inventory/Inventory.h"
#include "World.h"

class BreakTool : public NetworkItemBase 
{
	private:
	public:
	float reach;
	void onUsePress(unsigned char usage) override;
	BreakTool();
};
