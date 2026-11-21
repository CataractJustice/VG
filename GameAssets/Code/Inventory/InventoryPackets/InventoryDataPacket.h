#pragma once
#pragma pack(push, 1)
struct InventoryDataPacket 
{
	unsigned char packetType;
	unsigned int invId;
};
#pragma pack(pop)