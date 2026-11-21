#include "GenerateChunk.h"
#include "Chunk.h"
#include "Math/ValueNoiseInterpolator.h"
#include "VoxelMaterials.h"
void generateHeight(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed) 
{
	glm::ivec3 chunkPos = index * chunkSize;
	ValueNoiseInterpolator2D heightInterpolatorMain;
	ValueNoiseInterpolator2D heightInterpolatorDetail;
	ValueNoiseInterpolator2D heightInterpolatorDetail2;
	heightInterpolatorDetail.reset(3, 3, chunkSize.x / 2, glm::ivec3(index.x*2, index.z*2, worldSeed + 0), glm::ivec3(1));
	heightInterpolatorDetail2.reset(5, 5, chunkSize.x / 4, glm::ivec3(index.x*4, index.z*4, worldSeed + 1), glm::ivec3(1));
	heightInterpolatorMain.reset(2, 2, chunkSize.x, glm::ivec3(index.x, index.z, worldSeed + 2), glm::ivec3(1));
	const float heightAmplitude = 8.0f;
	const float heightBase = 18.0f;
	for(int i = 0; i < chunkSize.x; i++) 
	{
		for(int k = 0; k < chunkSize.z; k++) 
		{
			float height = heightBase + heightInterpolatorMain.value() * heightAmplitude + heightInterpolatorDetail.value() * heightAmplitude * 0.4 + 
			heightInterpolatorDetail2.value() * heightAmplitude * 0.32;
			heightInterpolatorMain.nextY();
			heightInterpolatorDetail.nextY();
			heightInterpolatorDetail2.nextY();
			for(int j = 0; j < chunkSize.y; j++) 
			{	
				glm::ivec3 chunkSpace = glm::ivec3(i,j,k);
				glm::ivec3 worldSpace = chunkSpace + chunkPos;
				float depth = ((float)(worldSpace.y)) - height;
				float value = -depth;
				Voxel voxel;
				voxel.amount = glm::clamp(value, -1.0f, 1.0f) * voxelAmountMax;
				unsigned int surfaceId = VoxelMaterials::getInstance().getIdByName("short_grass");
				if(worldSpace.y < 24) surfaceId = VoxelMaterials::getInstance().getIdByName("sand");
				voxel.voxelTypeId = depth < -16.0f ? VoxelMaterials::getInstance().getIdByName("slate") : surfaceId;
				chunk->voxelData.quickSetVoxel(glm::ivec3(i,j,k), voxel);
			}
		}
		heightInterpolatorMain.nextX();
		heightInterpolatorDetail.nextX();
		heightInterpolatorDetail2.nextX();
	}
}