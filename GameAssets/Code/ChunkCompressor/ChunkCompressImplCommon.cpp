#include "ChunkCompressImplCommon.h"
void unpackCubeDimensions(unsigned short data, unsigned int& w, unsigned int& h, unsigned int& l) 
{
	w = ((data >> 0) & 0b11111) + 1;
	h = ((data >> 5) & 0b11111) + 1;
	l = ((data >>10) & 0b11111) + 1;
}
unsigned short packCubeDimensions(unsigned int w, unsigned int h, unsigned int l) 
{
	return 
	((w - 1u) << 0) | 
	((h - 1u) << 5) | 
	((l - 1u) <<10);
}
