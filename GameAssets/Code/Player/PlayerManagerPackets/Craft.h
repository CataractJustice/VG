#pragma once

#pragma pack(push, 1)
struct PlayerCraftByIndexPacket 
{
	unsigned char packetType;
	unsigned short craftIndex;
};
#pragma pack(pop)