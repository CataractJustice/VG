#include "GenerateChunk.h"
#include "Chunk.h"
void generateWater(Chunk* chunk, glm::ivec3 chunkIndex) 
{
	if(chunkIndex.y > 0) return;
	glm::ivec3 index;
	for(index.x = 0; index.x < 32; index.x++) 
	for(index.y = 0; index.y < 32; index.y++) 
	for(index.z = 0; index.z < 32; index.z++) 
	{
		if(
			chunk->voxelData.getVoxelConst(index).amount <= 0 &&
			chunk->tileMap.getTileAtIndex(index).cubeMaterialId == 0
		) chunk->fluidSim.setCell(index, 255, 1, false);
	}
}