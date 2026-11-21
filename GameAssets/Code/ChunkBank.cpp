#include "ChunkBank.h"

ChunkBank::ChunkBank() 
{
	const glm::ivec3 dimensions = glm::ivec3(32);
	this->chunks = static_cast<Chunk*>(
		operator new[](chunkBankSize * sizeof(Chunk))
	);

	for (int i = 0; i < chunkBankSize; ++i) {
		new (&chunks[i]) Chunk(dimensions, glm::ivec3(0));
		freeSpace[i] = i;
	}
	freeSpaceHead = chunkBankSize;
}

void resetChunkData(Chunk* chunk) 
{
	chunk->entities.clear();
	chunk->frustumCulling = false;
	chunk->terrainGenerated = false;
	chunk->dirtyMesh = false;
	chunk->dirtyTileMesh = false;
	chunk->mesh->markWholeChunkDirty();
}

void ChunkBank::freeChunk(Chunk* chunk) 
{
	if(freeSpaceHead >= chunkBankSize) throw std::underflow_error("Freed chunks with full free chunk stack");
	freeSpace[freeSpaceHead++] = chunk - chunks;

	freeingMutex[chunk - chunks].lock();
	resetChunkData(chunk);
	chunk->isFreed = true;
	freeingMutex[chunk - chunks].unlock();
	
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	for(int k = -1; k <= 1; k++) 
	{
		chunk->setNeighbor(glm::ivec3(i,j,k), nullptr);
		chunk->voxelData.chunk = chunk;
		chunk->voxelData.setNeighbor(glm::ivec3(i,j,k), nullptr);
	}
}

Chunk* ChunkBank::newChunk() 
{
	if(freeSpaceHead <= 0) throw std::overflow_error("Too many chunks used");
	unsigned short freeId = freeSpace[--freeSpaceHead];
	Chunk* chunk = &chunks[freeId];
	freeingMutex[freeId].lock();
	resetChunkData(chunk);
	chunk->isFreed = false;
	freeingMutex[freeId].unlock();
	chunk->mesh->clearAll();
	chunk->voxelData.chunk = chunk;
	return chunk;
}

bool ChunkBank::freeLock(Chunk* chunk) 
{
	freeingMutex[chunk - chunks].lock_shared();
	if(chunk->isFreed) 
	{
		freeingMutex[chunk - chunks].unlock_shared();
		return false;
	}
	return true;
}
void ChunkBank::freeUnlock(Chunk* chunk) 
{
	freeingMutex[chunk - chunks].unlock_shared();
}