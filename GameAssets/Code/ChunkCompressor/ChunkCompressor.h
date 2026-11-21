#pragma once
#include "libs/glm/glm.hpp"
#include "Chunk.h"
class ChunkCompressor 
{
	private:
	char compressionBuffer[2097152];
	unsigned int voxelTypeVisited[1024]; 
	unsigned int voxelAmountVisited[1024]; 
	unsigned int writeCursor;
	void write(const void* data, unsigned int size);
	void setBit(unsigned int* bitmap, unsigned int x, unsigned int y, unsigned int z, bool value) 
	{
		bitmap[y + x * 32] |= 1u << z;
	}
	bool getBit(unsigned int* bitmap, unsigned int x, unsigned int y, unsigned int z) 
	{
		return bitmap[y + x * 32] & 1u << z;
	}
	template<class T>
	void write(const T& data) 
	{
		write(&data, sizeof(T));
	} 
	void clear();
	static void surfaceCheck(const VoxelData& voxels, int x, int y, int z, bool& nearAir, bool& nearSolid, unsigned int flatIndex);
	static bool canStoreWater(const Chunk* chunk, unsigned int flatIndex);
	public: 
	char* compressChunk(Chunk* chunk, unsigned int& size);
	void decompressChunk(Chunk* chunk, const char* data, unsigned int size);
	char* fastCompressChunk(Chunk* chunk, unsigned int& size);
	void fastDecompressChunk(Chunk* chunk, const char* data, unsigned int size);
};