#include "GenerateChunk.h"
#include "World.h"
#include "Math/ValueNoiseInterpolator.h"

void generateBlobs(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed) 
{
	ValueNoiseInterpolator3D blobs;
	ValueNoiseInterpolator2D blobsMultiplier;
	glm::ivec3 noiseindex = glm::ivec3(index.x, index.z, index.y);
	glm::ivec3 caveOrigin = noiseindex;
	blobsMultiplier.reset(2, 2, chunkSize.x, glm::ivec3(index.x, index.z, 5), glm::ivec3(1, 1, 0));
	const int blobSquish = 4;
	blobs.reset(3, 3, 1 + (chunkSize.y / chunkSize.x) * blobSquish * 2, chunkSize.x / 2, glm::ivec3(index.x*2, index.z*2, index.y), glm::ivec3(1));
	const float blobForce = 42.0f;
	for(int i = 0; i < chunkSize.x; i++) 
	{
		for(int k = 0; k < chunkSize.z; k++) 
		{
			const float bMult = glm::pow(1.0f - glm::abs(blobsMultiplier.value() - 1.0f) * 2.0f, 2.0f);
			blobsMultiplier.nextY();
			for(int j = 0; j < chunkSize.y; j++) 
			{
				const float blobValue = (blobs.value() * 2.0f - 1.0f)  * bMult;
				for(int s = 0; s < blobSquish; s++) 
				{
					blobs.nextZ();
				}
				glm::ivec3 chunkSpace = glm::ivec3(i,j,k);
				Voxel voxel = chunk->voxelData.getVoxelConst(chunkSpace);
				voxel.amount = glm::clamp(float(voxel.amount) / blobForce + glm::clamp((blobValue - (float(j) + float(index.y) * float(chunkSize.y) - 24.0f) / 24.0f) * (voxelAmountScale), -voxelAmountScale, voxelAmountScale), -voxelAmountScale, voxelAmountScale);
				chunk->voxelData.quickSetVoxel(glm::ivec3(i,j,k), voxel);
			}
			blobs.nextY();
		}
		blobsMultiplier.nextX();
		blobs.nextX();
	}
}