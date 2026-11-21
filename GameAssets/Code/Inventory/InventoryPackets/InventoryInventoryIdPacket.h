#pragma once
#pragma pack(push, 1)
struct InventoryInventoryIdPacket 
{
	unsigned char packetType;
	unsigned int id;
	char name[128];
};
#pragma pack(pop)