#pragma once
#include "GameNode/Component.h"
#include "Player/PlayerManagerPackets/UseItem.h"
#include "EquippedItem.h"
using namespace TealEngine;

const float itemUsageSyncRange = 320.0f;

class NetworkItemBase : public Component 
{
	public:
	NetworkId playerManagerNetId;
	static const unsigned int keepHoldingTicks = 240;
	static const unsigned int holdingTicksRefreshAt = 100;
	virtual void processPacketHost(const void* data, unsigned int size);
	virtual void processPacketClient(const void* data, unsigned int size);
	virtual void onUsePress(unsigned char usageType);
	virtual void onUseRelease(unsigned char usageType);
	virtual void onUseHoldTick(unsigned char usageType);
	virtual void updateItem();

	void onWindowMousePress(unsigned short button) override;
	void onWindowMouseRelease(unsigned short button) override;
	void doPress(unsigned char usageType);
	void doRelease(unsigned char usageType);
	void holdRefresh(unsigned char usageType);
	void update() override;
	void onAttach() override;
	bool isUsageHeld(unsigned char usage);
	NetworkItemBase();
	protected:
	EquippedItem* item;
	private:
	bool shouldSendPacket();
	unsigned int heldUsages[ItemUsageType::maxUsageType];
	void sync(unsigned char usage, unsigned char update, unsigned int playerId);
};
