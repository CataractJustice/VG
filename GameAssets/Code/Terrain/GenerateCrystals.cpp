#include "GenerateChunk.h"
#include "Math/Noise.h"
#include "Math/ValueNoiseInterpolator.h"
#include "World.h"
#include "VoxelMaterials.h"
void generateCrystals(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed) 
{
	unsigned short crystalId = VoxelMaterials::getInstance().getIdByName("mana_crystal");
	if(Noise::uintGet(index.x, index.z, worldSeed) % 12 == 0) 
	{
		float r = (Noise::get(index.x, index.z, worldSeed) * 0.2f + 0.1f) * chunkSize.x;
		float h = Noise::get(index.x, worldSeed, index.z) * 48.0f;
		float s = Noise::get(worldSeed, index.x, index.z);
		TealEngine::ValueNoiseInterpolator3D noise;
		noise.reset(17, 9, 17, chunkSize.x / 8, index * 12, glm::ivec3(1));
		for(int i = 0; i < chunkSize.x; i++) 
		{
			for(int j = 0; j < chunkSize.y; j++) 
			{
				for(int k = 0; k < chunkSize.z; k++) 
				{
					float n = noise.value();
					noise.nextZ();
					noise.nextZ();

					float val = (1.0f - glm::length(glm::vec3(i - chunkSize.x * 0.5f, (h - index.y * chunkSize.y - j) * (0.2f + s * 0.5f), k - chunkSize.z * 0.5f)) / r) - n * 0.5f;

					Voxel v = chunk->voxelData.getVoxelConst(glm::ivec3(i,j,k));
					if(v.amount < val * voxelAmountScale) 
					{
						v.amount = val > 0.0f ? voxelAmountMax : voxelAmountMin;
						v.voxelTypeId = crystalId;
					}
					chunk->voxelData.quickSetVoxel(glm::ivec3(i,j,k), v);
				}
				noise.nextY();
			}
			noise.nextX();
			noise.nextX();
		}
	}
}