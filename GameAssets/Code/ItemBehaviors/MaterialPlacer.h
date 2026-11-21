#pragma once
#include "EquippedItem.h"
#include "NetworkItemBase.h"
class MaterialPlacer : public NetworkItemBase 
{
	private:
	float reach;
	float radius;
	float speed;
	public:

	void onUsePress(unsigned char usage) override;
	MaterialPlacer();
};
