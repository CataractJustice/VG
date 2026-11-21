#include "GenerateChunk.h"
#include "Chunk.h"

void generateCapsuleLine(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, glm::vec4 from, glm::vec4 to, int typeID) 
{
	float startRadius = from.w;
	float endRadius = to.w;
	float maxRadius = glm::max(startRadius, endRadius);
	glm::vec3 from3 = from;
	glm::vec3 to3 = to;
	glm::vec3 fromLoc = from3 - glm::vec3(index * chunkSize);
	glm::vec3 toLoc = to3 - glm::vec3(index * chunkSize);
	glm::vec3 delta = to3 - from3;
	float Len = glm::length(delta);
	glm::vec3 dir = delta / Len;
	glm::ivec3 fromInd = glm::ivec3(fromLoc);
	glm::ivec3 toInd = glm::ivec3(toLoc);
	glm::ivec3 minInd = glm::min(fromInd, toInd) - ((int)maxRadius) - 1;
	glm::ivec3 maxInd = glm::max(fromInd, toInd) + ((int)maxRadius) + 1;
	minInd = glm::max(glm::ivec3(0), minInd);
	maxInd = glm::min(chunkSize, maxInd);
	
	for(int i = minInd.x; i < maxInd.x; i++) 
	{
		for(int j = minInd.y; j < maxInd.y; j++) 
		{
			for(int k = minInd.z; k < maxInd.z; k++) 
			{
				glm::vec3 loc = glm::vec3(i,j,k);
				glm::vec3 glob = glm::vec3(index * chunkSize) + loc;
				float l = glm::dot(dir, glob - from3) / Len;
				float radius = glm::mix(startRadius, endRadius, glm::clamp(l, 0.0f, 1.0f));
				float distance = 0.0f;
				if(l < 0.0f) distance = glm::distance(loc, fromLoc);
				else
				if(l > 1.0f) distance = glm::distance(loc, toLoc);
				else
				distance = glm::length(glob - from3 - delta * l);
				float val = glm::clamp((radius - distance) / radius, -1.0f, 1.0f);
				Voxel newVoxel;
				Voxel oldVoxel = chunk->voxelData.getVoxelConst(glm::ivec3(i,j,k));
				newVoxel.amount = val * voxelAmountMax;
				newVoxel.voxelTypeId = newVoxel.amount >= oldVoxel.amount ? typeID : oldVoxel.voxelTypeId;
				newVoxel.amount = glm::max(newVoxel.amount, oldVoxel.amount);
				chunk->voxelData.quickSetVoxel(glm::ivec3(i,j,k), newVoxel);
			}	
		}	
	}
}