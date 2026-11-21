#pragma once
#pragma pack(push, 1)
struct InventorySlotTryMovePacket 
{
	unsigned char packetType;
	unsigned int slotFrom;
	unsigned int slotTo;
	unsigned int amount;
};
#pragma pack(pop)