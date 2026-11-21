#pragma once
#pragma pack(push, 1)
struct EntityPacket 
{
	unsigned char packetType;
	unsigned long long entityId;
};
#pragma pack(pop)