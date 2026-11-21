#include "GenerateChunk.h"
#include "Math/Noise.h"
#include "Chunk.h"
#include "Math/PerlinNoise.h"
void generateTrees(Chunk* chunk, glm::ivec3 index, glm::ivec3 realIndex, glm::ivec3 chunkSize, int worldSeed, glm::ivec3 offset) 
{
	const unsigned int treesSeed = Noise::uintGet(worldSeed, index.x, index.z);
	float density = PerlinNoise::get(((float)index.x) / 5.0f, 0.0f, ((float)index.z) / 5.0f, 2);
	density *= 2.0f;
	density -= 1.0f;
	density = glm::abs(glm::pow(density, 0.5f)) * glm::sign(density);
	const int treeCount = (density) * 3; 

	//generateCapsuleLine(chunk, index, chunkSize, glm::vec4(index*chunkSize, 4.0f), glm::vec4(index*chunkSize+16, 4.0f), 3);

	for(int treeInd = 0; treeInd < treeCount; treeInd++) 
	{
		int treeX = Noise::uintGet(treesSeed, index.x * treeInd, index.z);
		int treeZ = Noise::uintGet(-index.z, index.x, treesSeed * treeInd);
		float treeHeight = Noise::get(index.z, index.x * worldSeed, treeInd) * 4.0f + 8.0f;
		float treeRadius = Noise::get(treeInd, treesSeed, worldSeed) * 2 + 1.0f;
		treeX %= chunkSize.x;
		treeZ %= chunkSize.z;
		glm::vec3 treePos = glm::vec3((float)(treeX + index.x * chunkSize.x), 8.0f, (float)(treeZ + index.z * chunkSize.z));
		bool goUp = getGeneratorTerrainValueAt(treePos, worldSeed) > 0.0f;
		for(int emptyGroundedSpotSearchIterations = 0; emptyGroundedSpotSearchIterations <= 64; emptyGroundedSpotSearchIterations++) 
		{
			if(goUp) 
			{
				treePos.y += 5.0f;
				if(getGeneratorTerrainValueAt(treePos, worldSeed) <= 0.1f) break; 
			}
			else 
			{
				treePos.y -= 5.0f;
				if(getGeneratorTerrainValueAt(treePos, worldSeed) > 0.1f) break; 
			}
		}
		const float treeBaseHeight = treePos.y;
		glm::vec3 wpos = glm::vec3(chunkSize * glm::ivec3(index.x, 0, index.z) + glm::ivec3(treeX, 0, treeZ) + offset);
		generateFractalTree(chunk, realIndex, chunkSize, wpos + glm::vec3(0.0f, treeBaseHeight, 0.0f), glm::vec3(0.0f, treeHeight, 0.0f), 1.8f, 0.8f, 0.75f, 1.5f, 3, treesSeed + treeInd);
	}
	/*
	for(int treeInd = 0; treeInd < treeCount; treeInd++) 
	{
		int treeX = Noise::uintGet(treesSeed, index.x * treeInd, index.z);
		int treeZ = Noise::uintGet(-index.z, index.x, treesSeed * treeInd);
		int treeHeight = Noise::uintGet(index.z, index.x * worldSeed, treeInd);
		unsigned int treeRadius = Noise::uintGet(treeInd, treesSeed, worldSeed);
		treeRadius %= 2;
		treeRadius += 1;
		treeHeight %= 16;
		treeHeight += 20;
		const int treeBaseHeight = 28;
		float treeRadiusFract = Noise::get(treesSeed, treeInd, worldSeed);
		treeX %= chunkSize.x;
		treeZ %= chunkSize.z;
		for(int i = glm::max(0, (int)treeX - (int)treeRadius + offset.x); i < glm::min(chunkSize.x, (int)treeX + (int)treeRadius + 1 + offset.x); i++) 
		{
			for(int k = glm::max(0, (int)treeZ - (int)treeRadius + offset.z); k < glm::min(chunkSize.z, (int)treeZ + (int)treeRadius + 1 + offset.z); k++) 
			{
				for(int j = 0; j < chunkSize.y; j++) 
				{
					Voxel oldVoxel = *chunk->voxelData.getVoxelConst(glm::ivec3(i,j,k));
					Voxel newVoxel;
					const int worldY = index.y * chunkSize.y + j;
					const int treeY = worldY - treeBaseHeight;

					if(oldVoxel.amount > 0) newVoxel.voxelTypeId = oldVoxel.voxelTypeId;
					else
					newVoxel.voxelTypeId = 4; 
					float val = (((float)(treeRadius)) - glm::length(glm::vec2(i, k) - glm::vec2(treeX + offset.x, treeZ + offset.z))) - treeRadiusFract;
					val /= (float)(treeRadius);
					val -= (float)treeY / (float)treeHeight;
					val = glm::clamp(val, -1.0f, 1.0f);
					newVoxel.amount = val * 32000;
					newVoxel.amount = glm::max(newVoxel.amount, oldVoxel.amount);
					chunk->voxelData.quickSetVoxel(glm::ivec3(i,j,k), newVoxel);
				}
			}
		}
	}*/
}