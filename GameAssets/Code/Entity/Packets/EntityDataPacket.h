#pragma once
#pragma pack(push, 1)
struct EntityDataPacket 
{
	unsigned char packetType;
	unsigned long long entityId;
	unsigned int entityType;
};
#pragma pack(pop)