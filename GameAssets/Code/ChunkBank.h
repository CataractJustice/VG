#pragma once
#include "Chunk.h"
#include "System/StaticSizeArray.h"
const unsigned int chunkBankSize = 1024;
class ChunkBank 
{
	private:
	Chunk* chunks;
	unsigned short freeSpace[chunkBankSize];
	std::shared_mutex freeingMutex[chunkBankSize];
	unsigned short freeSpaceHead;
	public:
	Chunk* newChunk();
	void freeChunk(Chunk*);
	bool freeLock(Chunk*);
	void freeUnlock(Chunk*);
	ChunkBank();
};