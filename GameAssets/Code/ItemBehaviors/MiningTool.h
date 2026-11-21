#include "Inventory/Inventory.h"
#include "World.h"
#include "NetworkItemBase.h"

class MiningTool : public NetworkItemBase 
{
	private:
	float reach;
	float radius;
	
	float heat;
	float heatingSpeed;
	float heatDissipation;
	float laserBeamCharge;
	float laserBeamChargeSpeed;
	
	int breakCD;
	int breakCDTimer;

	const Kirino::Material* heatingMaterial;
	GameNode* beamHeatupModel;
	GameNode* beamModel;
	public:

	void onUseHoldTick(unsigned char usage) override;
	void updateItem();
	MiningTool();
};
