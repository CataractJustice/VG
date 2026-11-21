#pragma once
#pragma pack(push, 1)

namespace ItemUsageType 
{
	const unsigned char primary = 0;
	const unsigned char secondary = 1;
	const unsigned char maxUsageType = 2;
}

namespace ItemUpdateType 
{
	const unsigned char press = 0;
	const unsigned char release = 1;
	const unsigned char hold = 2;
	const unsigned char packet = 3;
}

struct PlayerUseItemPacket 
{
	unsigned char packetType;
	unsigned char hotbarIndex;
	unsigned char usageType;
	unsigned char updateType;
	unsigned int peerId;
};
#pragma pack(pop)