#pragma once
#pragma pack(push, 1)
struct EntityCreatePacket 
{
	unsigned char packetType;
	unsigned int entityType;
	unsigned long long entityId;
};
#pragma pack(pop)