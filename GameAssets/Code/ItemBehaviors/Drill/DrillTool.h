#include "ItemBehaviors/NetworkItemBase.h"
#include "GameNode/GameNode3D.h"
class FollowNode;
class DrillTool : public NetworkItemBase 
{
	private:
	GameNode3D* drillIdlePosition;
	GameNode3D* drillModel;
	GameNode3D* drillBitModel;
	GameNode3D* drillActivePosition;
	GameNode3D* drillBaseHinge;
	GameNode3D* firstSmallHingeFirstPartAnchor;
	GameNode3D* firstSmallHingeSecondPartAnchor;
	GameNode3D* firstSmallHingeFirstPart;
	GameNode3D* firstSmallHingeSecondPart;
	GameNode3D* secondHingeAnchor;
	GameNode3D* secondHinge;
	FollowNode* followComp;
	unsigned int activeTicks;
	float drillBaseHingeLength;
	float drillBaseHingeOffset;
	float range;
	float drillModelSurfaceDistance;
	float rotationSpeed;
	float rotationAcceleration;
	float rotationMaxSpeed;
	public:
	DrillTool();
	void onUseHoldTick(unsigned char usage) override;
	void updateItem();
	void onAttach();
};