#include "GenerateChunk.h"
#include "Math/ValueNoiseInterpolator.h"
#include "Math/PerlinNoise.h"
using namespace TealEngine;
void generateMonoliths(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed) 
{
	/*
	ValueNoiseInterpolator2D monolithDensity;
	const int noiseScale = 4;
	monolithDensity.reset(3, 3, chunkSize.x * noiseScale, (index / noiseScale) + glm::ivec3(index.x < 0, index.y < 0, index.z < 0), glm::ivec3(1));
	const glm::ivec3 subSampleIndex = index + chunkSize 
	*/
	for(int i = 0; i < chunkSize.x; i++)
	for(int j = 0; j < chunkSize.y; j++)
	for(int k = 0; k < chunkSize.z; k++) 
	{
		
	}
}