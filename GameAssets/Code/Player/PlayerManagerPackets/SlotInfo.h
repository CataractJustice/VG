#pragma once

namespace PlayerSlotInfoPacketSlotType 
{
	const unsigned char hotbar = 0;
	const unsigned char backpack = 1;
	const unsigned char container = 2;
}

const unsigned char notSubSlot = 255;

#pragma pack(push, 1)
struct PlayerGetSlotInfoPacket
{
	unsigned char packetType;
	unsigned char slotType;
	unsigned char subSlotIndex;
	unsigned int slotIndex;
	unsigned long long containerId;
};

struct PlayerSlotInfoPacket 
{
	unsigned char packetType;
	unsigned char slotType;
	unsigned char subSlotIndex;
	unsigned int slotIndex;
	unsigned long long containerId;
};	
#pragma pack(pop)