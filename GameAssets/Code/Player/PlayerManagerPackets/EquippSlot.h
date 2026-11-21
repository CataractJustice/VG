#pragma once

#pragma pack(push, 1)
struct PlayerEquippSlotPacket 
{
	unsigned char packetType;
	unsigned char slot;
};
#pragma pack(pop)