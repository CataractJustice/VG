#include "Chunk.h"
static const unsigned int width = 32;
static const unsigned int width2 = width*width;
static const unsigned short packedCubeNewTypeFlag = 0b1000000000000000;
static const unsigned int surfaceCheckCount = 0b111;
void unpackCubeDimensions(unsigned short data, unsigned int& w, unsigned int& h, unsigned int& l);
unsigned short packCubeDimensions(unsigned int w, unsigned int h, unsigned int l);

const unsigned int ChunkDataSaveMajikV2 = 0xfee71eec;

namespace ChunkDataHeaderV2Flags 
{
	static const unsigned char hasWater = 1;
	static const unsigned char hasTiles = 2;
};

#pragma pack(push, 1)
struct ChunkDataVersionHeader 
{
	unsigned int majik;
	unsigned int version;
};

struct ChunkDataHeaderV2 
{
	unsigned char flags;
	unsigned long long lastModify;
};
#pragma pack(pop)


const unsigned int currentVersion = 5;