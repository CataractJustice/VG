#pragma once
#pragma pack(push, 1)
struct ChunkDataPacket
{
	unsigned char packetType;
	int x;
	int y;
	int z;
};
#pragma pack(pop)
