#include "GenerateChunk.h"
#include "World.h"
#include "Math/Noise.h"
void generateOres(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, unsigned short id, unsigned short replaceId, unsigned int chunksPerSpawn, unsigned int size,  int worldSeed) 
{
	unsigned int oreSeeds[27];
	const int sizeSquare = size * size;

	for(int i = -1; i <= 1; i++) 
	for(int j = -1; j <= 1; j++) 
	for(int k = -1; k <= 1; k++) 
	{
		unsigned int seedIndex = (k + 1) + (j + 1) * 3 + (i + 1) * 9;
		glm::ivec3 oreChunkIndex = index + glm::ivec3(i,j,k);
		oreSeeds[seedIndex] = Noise::uintGet(
			oreChunkIndex.x ^ (worldSeed * 17) ^ (id * 9999991),
			oreChunkIndex.y ^ (worldSeed * 13) ^ (id * 99991),
			oreChunkIndex.z ^ (worldSeed * 23) ^ (id * 991)
		);
	}

	for(int i = -1; i <= 1; i++) 
	for(int j = -1; j <= 1; j++) 
	for(int k = -1; k <= 1; k++) 
	{
		glm::ivec3 oreChunkIndex = index + glm::ivec3(i,j,k);
		unsigned int seedIndex = (k + 1) + (j + 1) * 3 + (i + 1) * 9;
		unsigned int seed = oreSeeds[seedIndex];
		if(seed % chunksPerSpawn) 
		{
			continue;
		}
		glm::ivec3 oreOrigin = glm::ivec3(
			seed % chunkSize.x, 
			(seed / chunkSize.x) % chunkSize.y, 
			(seed / chunkSize.x / chunkSize.y) % chunkSize.z
		);
		oreOrigin += glm::ivec3(i,j,k) * chunkSize;
		for(int x = std::max(oreOrigin.x-int(size), 0); x <= std::min(oreOrigin.x+int(size), chunkSize.x - 1); x++)
		for(int y = std::max(oreOrigin.y-int(size), 0); y <= std::min(oreOrigin.y+int(size), chunkSize.y - 1); y++)
		for(int z = std::max(oreOrigin.z-int(size), 0); z <= std::min(oreOrigin.z+int(size), chunkSize.z - 1); z++)
		{
			glm::ivec3 cellIndex = glm::ivec3(x,y,z);
			glm::ivec3 relativeIndex = cellIndex - oreOrigin;
			int square = relativeIndex.x * relativeIndex.x + relativeIndex.y * relativeIndex.y + relativeIndex.z * relativeIndex.z;
			if(square <= sizeSquare) 
			{
				const Voxel currentVoxel = chunk->voxelData.getVoxelConst(cellIndex);
				if(currentVoxel.voxelTypeId == replaceId) 
				{
					Voxel v;
					v.amount = currentVoxel.amount;
					v.voxelTypeId = id;
					chunk->voxelData.quickSetVoxel(cellIndex, v);
				}
			}
			else if(relativeIndex.z > 0) 
			{
				break;
			}
		}
	}
}