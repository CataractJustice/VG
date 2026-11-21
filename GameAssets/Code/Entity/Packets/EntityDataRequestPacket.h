#pragma once
#pragma pack(push, 1)
struct EntityDataRequestPacket 
{
	unsigned char packetType;
	unsigned long long entityId;
};
#pragma pack(pop)