#pragma once
#pragma pack(push, 1)
struct InventoryRequestInventoryIdPacket 
{
	unsigned char packetType;
	char name[128];
};
#pragma pack(pop)