#include "World.h"
#include "Math/ValueNoiseInterpolator.h"
#include "GenerateChunk.h"
#include "VoxelMaterials.h"
void generateChunk(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed) 
{
	static const unsigned short slateId = VoxelMaterials::getInstance().getIdByName("slate");
	static const unsigned short ironOreId = VoxelMaterials::getInstance().getIdByName("iron_ore");
	static const unsigned short coalId = VoxelMaterials::getInstance().getIdByName("coal");
	chunk->voxelData.setVoxelNeighborCounting(false);
	glm::ivec3 chunkPos = index * chunkSize;
	if(index.y >= -1) 
	{
		generateTerrain(chunk, index, chunkSize, worldSeed);
		generateBoulders(chunk, index, chunkSize, worldSeed);
		generateCrystals(chunk, index, chunkSize, worldSeed);
	}
	else
	{
		generateFill(chunk, VoxelMaterials::getInstance().getIdByName("slate"), voxelAmountMax);
	}
	generateWater(chunk, index);

	generateCaves(chunk, index, chunkSize, worldSeed);
	if(index.y < 8) 
	{
		unsigned int oreSeed = worldSeed + 0xfee7fee7;
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 1, 2, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 1, 2, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 1, 2, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 1, 2, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 1, 2, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 1, 2, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 1, 2, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 1, 2, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 2, 3, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 2, 3, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 2, 3, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 2, 3, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 2, 3, oreSeed++);
		generateOres(chunk, index, chunkSize, ironOreId, slateId, 3, 4, oreSeed++);
		generateOres(chunk, index, chunkSize, coalId, slateId, 1, 4, oreSeed++);
		generateOres(chunk, index, chunkSize, coalId, slateId, 1, 4, oreSeed++);
		generateOres(chunk, index, chunkSize, coalId, slateId, 1, 4, oreSeed++);
		generateOres(chunk, index, chunkSize, coalId, slateId, 1, 4, oreSeed++);
		generateOres(chunk, index, chunkSize, coalId, slateId, 1, 4, oreSeed++);
		generateOres(chunk, index, chunkSize, coalId, slateId, 1, 4, oreSeed++);
		generateOres(chunk, index, chunkSize, coalId, slateId, 1, 3, oreSeed++);
		generateOres(chunk, index, chunkSize, coalId, slateId, 2, 5, oreSeed++);
	}
	if(index.y >= 0 && index.y < 3) 
	{
		generateTrees(chunk, index+glm::ivec3( 0, 0, 0), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(0,0,0)*/);
		generateTrees(chunk, index+glm::ivec3( 1, 0, 0), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(1,0,0)*/);
		generateTrees(chunk, index+glm::ivec3( 0, 0, 1), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(0,0,1)*/);
		generateTrees(chunk, index+glm::ivec3( 1, 0, 1), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(1,0,1)*/);
		generateTrees(chunk, index+glm::ivec3(-1, 0, 0), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(1,0,0)*/);
		generateTrees(chunk, index+glm::ivec3( 0, 0,-1), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(0,0,1)*/);
		generateTrees(chunk, index+glm::ivec3(-1, 0,-1), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(1,0,1)*/);
		generateTrees(chunk, index+glm::ivec3( 1, 0,-1), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(1,0,1)*/);
		generateTrees(chunk, index+glm::ivec3(-1, 0, 1), 	index, chunkSize, worldSeed, glm::ivec3(0) /*+ chunkSize * glm::ivec3(1,0,1)*/);
	}
	

	//chunk->voxelData.setVoxelNeighborCounting(true);
	//chunk->voxelData.recountVoxelNeighbors();
	//chunk->voxelData.flattenData();
	/*
	if(true) 
	{
		ValueNoiseInterpolator2D heightInterpolatorMain;
		ValueNoiseInterpolator2D heightInterpolatorDetail;
		ValueNoiseInterpolator2D heightInterpolatorDetail2;
		ValueNoiseInterpolator3D blobs;
		ValueNoiseInterpolator2D blobsMultiplier;
		ValueNoiseInterpolator3D cavesNoise0;
		ValueNoiseInterpolator3D cavesNoise1;
		ValueNoiseInterpolator3D caveValueNoise;
		glm::ivec3 noiseindex = glm::ivec3(index.x, index.z, index.y);
		glm::ivec3 caveOrigin = noiseindex;
		cavesNoise0.reset(3,3,3,chunkSize.x/2, caveOrigin*2, glm::ivec3(1));
		cavesNoise1.reset(3,3,3,chunkSize.x/2, caveOrigin*4 + 4096, glm::ivec3(2));
		caveValueNoise.reset(2, 2, 2, chunkSize.x, caveOrigin, glm::ivec3(1));
		heightInterpolatorDetail.reset(3, 3, chunkSize.x / 2, glm::ivec3(chunkPos.x, chunkPos.z, 0), glm::ivec3(chunkSize.x / 2, chunkSize.x / 2, 0));
		heightInterpolatorDetail2.reset(5, 5, chunkSize.x / 4, glm::ivec3(chunkPos.x, chunkPos.z, 1), glm::ivec3(chunkSize.x / 4, chunkSize.x / 4, 0));
		heightInterpolatorMain.reset(2, 2, chunkSize.x, glm::ivec3(chunkPos.x, chunkPos.z, 2), glm::ivec3(chunkSize.x, chunkSize.x, 0));
		blobsMultiplier.reset(2, 2, chunkSize.x, glm::ivec3(index.x, index.z, 5), glm::ivec3(1, 1, 0));
		const int blobSquish = 4;
		blobs.reset(3, 3, 1 + (chunkSize.y / chunkSize.x) * blobSquish * 2, chunkSize.x / 2, glm::ivec3(index.x*2, index.z*2, index.y), glm::ivec3(1));
		const float heightAmplitude = 8.0f;
		const float heightBase = 18.0f;
		const float blobForce = 32.0f;
		for(int i = 0; i < chunkSize.x; i++) 
		{
			for(int k = 0; k < chunkSize.z; k++) 
			{
				float height = heightBase + heightInterpolatorMain.value() * heightAmplitude + heightInterpolatorDetail.value() * heightAmplitude * 0.5 + 
				heightInterpolatorDetail2.value() * heightAmplitude * 0.25;
				const float bMult = glm::pow(1.0f - glm::abs(blobsMultiplier.value() - 1.0f) * 2.0f, 2.0f);
				blobsMultiplier.nextY();
				heightInterpolatorMain.nextY();
				heightInterpolatorDetail.nextY();
				heightInterpolatorDetail2.nextY();
				for(int j = 0; j < chunkSize.y; j++) 
				{
					const float blobValue = (blobs.value() * 2.0f - 1.0f)  * bMult;
					for(int s = 0; s < blobSquish; s++) 
					{
						blobs.nextZ();
					}
					float cave0val = 1.0f - glm::abs(cavesNoise0.value() * 2.0f - 1.0f);
					float cave1val = 1.0f - glm::abs(cavesNoise1.value() * 2.0f - 1.0f);
					float caveVal = caveValueNoise.value();
					cavesNoise0.nextZ();
					cavesNoise1.nextZ();
					caveValueNoise.nextZ();
					glm::ivec3 chunkSpace = glm::ivec3(i,j,k);
					glm::ivec3 worldSpace = chunkSpace + chunkPos;
					float depth = ((float)(worldSpace.y)) - height;
					float value = -depth / blobForce + blobValue;
					value = value / 2.0f - glm::pow(cave0val * cave1val * caveVal, 2.0f) * glm::abs(depth + 1.0f) / blobForce; 
					Voxel voxel;
					voxel.amount = glm::clamp(value, -1.0f, 1.0f) * 32000;
					voxel.voxelTypeId = 1;
					chunk->voxelData.quickSetVoxel(glm::ivec3(i,j,k), voxel);
				}
				blobs.nextY();
				cavesNoise0.nextY();
				cavesNoise1.nextY();
				caveValueNoise.nextY();
			}
			heightInterpolatorMain.nextX();
			heightInterpolatorDetail.nextX();
			heightInterpolatorDetail2.nextX();
			blobsMultiplier.nextX();
			blobs.nextX();
			cavesNoise0.nextX();
			cavesNoise1.nextX();
			caveValueNoise.nextX();
		}
	}*/
}