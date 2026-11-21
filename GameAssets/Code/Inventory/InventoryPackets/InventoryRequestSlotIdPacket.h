#pragma once
#pragma pack(push, 1)
struct InventoryRequestSlotIdPacket 
{
	unsigned char packetType;
	char name[128];
};
#pragma pack(pop)