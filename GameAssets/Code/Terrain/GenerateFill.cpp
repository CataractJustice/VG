#include "GenerateChunk.h"
#include "Chunk.h"
void generateFill(Chunk* chunk, unsigned short id, signed char amount) 
{
	for(int i = 0; i < chunk->voxelData.getDimensions().x; i++)
	for(int j = 0; j < chunk->voxelData.getDimensions().y; j++)
	for(int k = 0; k < chunk->voxelData.getDimensions().z; k++)
		chunk->voxelData.quickSetVoxel(glm::ivec3(i,j,k), {amount, id});
}