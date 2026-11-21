#include "ChunkCompressor.h"
#include "ChunkCompressImplCommon.h"
#include "Chunk.h"

void ChunkCompressor::write(const void* data, unsigned int size) 
{
	memcpy(compressionBuffer + writeCursor, (const char*)data, size);
	writeCursor += size;
}

void ChunkCompressor::clear() 
{
	this->writeCursor = 0;
	memset(voxelTypeVisited, 0, sizeof(voxelTypeVisited));
}

void ChunkCompressor::surfaceCheck(const VoxelData& voxels, int x, int y, int z, bool& nearAir, bool& nearSolid, unsigned int flatIndex) 
{
	nearSolid = false;
	unsigned int flatCheckIndex = flatIndex - 1 - width - width2;
	for(int i = -1; i <= 1; i++)
	{
		if(x+i < 0 || x+i >= width) 
		{
			nearAir = true;
			flatCheckIndex += width2;
			continue;
		}
		for(int j = -1; j <= 1; j++) 
		{
			if(y+j < 0 || y+j >= width) 
			{
				nearAir = true;
				flatCheckIndex += width;
				continue;
			}
			for(int k = -1; k <= 1; k++) 
			{
				if(
					z + k < 0 || z + k >= width
				) 
				{
					nearAir = true;
					flatCheckIndex++;
					continue;
				}
				if(voxels.voxelTypeIds[flatCheckIndex]) nearSolid = true;
				else nearAir = true;

				if(nearAir && nearSolid) 
				{
					i = j = k = 2;
					break;
				}
				flatCheckIndex++;
			}
		}
	}
}

bool ChunkCompressor::canStoreWater(const Chunk* chunk, unsigned int flatIndex) 
{
	return chunk->voxelData.voxelTypeIds[flatIndex] == 0 && chunk->tileMap.cubeMaterials[flatIndex] == 0;
}