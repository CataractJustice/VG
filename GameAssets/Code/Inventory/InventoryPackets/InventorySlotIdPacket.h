#pragma once
#pragma pack(push, 1)
struct InventorySlotIdPacket 
{
	unsigned char packetType;
	char name[128];
	unsigned int id;
};
#pragma pack(pop)