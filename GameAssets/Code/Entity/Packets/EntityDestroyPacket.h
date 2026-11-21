#pragma once
#pragma pack(push, 1)
struct EntityDestroyPacket 
{
	unsigned char packetType;
	unsigned long long entityId;
};
#pragma pack(pop)