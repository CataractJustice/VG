#include "GenerateChunk.h"
#include "Math/Noise.h"
#include "Math/IndexMath.h"
#include "Math/IndexConvert.h"
#include "Chunk.h"

void carveCave(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, glm::vec4 from, glm::vec4 to) 
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
	glm::ivec3 minInd = glm::min(fromInd, toInd) - ((int)maxRadius) - 2;
	glm::ivec3 maxInd = glm::max(fromInd, toInd) + ((int)maxRadius) + 2;
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
				float val = glm::clamp((radius - distance) / radius, -1.0f, 1.0f) * 0.5f + 0.5f;
				val *= Noise::get(glm::ivec3(glob)) * 0.1f + 0.9f;
				Voxel v = chunk->voxelData.getVoxelConst(glm::ivec3(i,j,k));
				const float oldValue = (float)(v.amount) / voxelAmountScale;
				const float newValue = glm::mix(oldValue, -1.0f, val);
				v.amount = newValue * voxelAmountMax;
				chunk->voxelData.quickSetVoxel(glm::ivec3(i,j,k), v);
				
			}	
		}	
	}
}

const unsigned int cavesSeedOffset = 0xca8e5eed;

void generateCaves(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed) 
{
	const unsigned int caveChunkSize = 96;
	const unsigned int caveNodesCountBase = 32;
	float caveChunkScale = (float)caveChunkSize;
	glm::ivec3 caveSeedIndexOrigin = ivec3ToIndex(index*chunkSize, glm::ivec3(caveChunkSize));

	const float caveSegmentPositionDeltaRange = 8.2f;
	const float caveSegmentWidthDeltaRange = 1.5f;
	const unsigned int cavesPerCaveChunkBase = 4;
	for(int i = -2; i <= 2; i++)
	for(int j = -2; j <= 2; j++)
	for(int k = -2; k <= 2; k++) 
	{
		glm::ivec3 caveSeedIndex = caveSeedIndexOrigin + glm::ivec3(i,j,k);
		glm::vec3 caveChunkPosition = glm::vec3(caveSeedIndex) * caveChunkScale;
		const unsigned int cavesPerCaveChunk = cavesPerCaveChunkBase + glm::clamp(-caveSeedIndex.y * 2, 0, 32);
		const unsigned int caveNodesCount = caveNodesCountBase + glm::clamp(-caveSeedIndex.y, 0, 32);
		const float baseWidth = 1.8f + glm::clamp(-(float)caveSeedIndex.y * 0.1f, 0.0f, 0.6f);
		for(int caveId = 0; caveId < cavesPerCaveChunk; caveId++) 
		{
			unsigned int caveSeed = Noise::uintGet(caveSeedIndex.x + caveId, caveSeedIndex.y, caveSeedIndex.z, (worldSeed + 0xca1e5eed) ^ caveId);

			glm::vec4 caveLocalOrigin = glm::vec4(
				Noise::get(caveSeed, 0, worldSeed + 0) * caveChunkScale,
				Noise::get(caveSeed, 1, worldSeed + 0) * caveChunkScale,
				Noise::get(caveSeed, 2, worldSeed + 0) * caveChunkScale,
				Noise::get(caveSeed, 3, worldSeed + 0) * 2 + 1.0f
			);
 			glm::vec4 caveNodeFrom = caveLocalOrigin + glm::vec4(caveChunkPosition, 0.0f);

			glm::vec4 caveDelta = glm::vec4(
				Noise::get(caveSeed, 0, worldSeed + 1) * caveSegmentPositionDeltaRange - caveSegmentPositionDeltaRange * 0.5f,
				Noise::get(caveSeed, 1, worldSeed + 1) * caveSegmentPositionDeltaRange - caveSegmentPositionDeltaRange * 0.5f,
				Noise::get(caveSeed, 2, worldSeed + 1) * caveSegmentPositionDeltaRange - caveSegmentPositionDeltaRange * 0.5f,
				Noise::get(caveSeed, 3, worldSeed + 1) * caveSegmentWidthDeltaRange - caveSegmentWidthDeltaRange * 0.5f
			);
			// = caveNodeFrom + caveDelta;
			for(int caveNodeId = 1; caveNodeId < caveNodesCount; caveNodeId++) 
			{
				glm::vec4 caveNodeTo = caveNodeFrom + caveDelta;
				caveDelta *= 0.5f;
				caveNodeTo.w = glm::mix(caveNodeTo.w, baseWidth, 0.45f);
				caveDelta += glm::vec4(
					Noise::get(caveSeed, 0, worldSeed + caveNodeId + 1) * (caveSegmentPositionDeltaRange) - caveSegmentPositionDeltaRange * 0.5f,
					Noise::get(caveSeed, 1, worldSeed + caveNodeId + 1) * (caveSegmentPositionDeltaRange) - caveSegmentPositionDeltaRange * 0.5f,
					Noise::get(caveSeed, 2, worldSeed + caveNodeId + 1) * (caveSegmentPositionDeltaRange) - caveSegmentPositionDeltaRange * 0.5f,
					Noise::get(caveSeed, 3, worldSeed + caveNodeId + 1) * (caveSegmentWidthDeltaRange) - caveSegmentWidthDeltaRange * 0.5f
				);
				carveCave(chunk, index, chunkSize, caveNodeFrom, caveNodeTo);
				caveNodeFrom = caveNodeTo;
			}
		}
	}
}