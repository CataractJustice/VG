#include "GenerateChunk.h"
#include "Math/Interpolation.h"
#include "Chunk.h"
#include "VoxelMaterials.h"
#include "Math/PerlinNoise.h"

float getVolume(glm::vec3 position, int worldSeed) 
{
	const float blobMultScale = 96.0f;
	const float blobScale = 12.0f;
	const float blobMult = glm::pow(PerlinNoise::get(position.x / blobMultScale, worldSeed + 0x600df00d, position.z / blobMultScale), 4.0f);
	const float blob = PerlinNoise::get(position.x / blobScale, position.y / blobScale * 2.0f, position.z / blobScale, 3, 0.65f);
	return (blob * 1.5f - 0.5f) * (blobMult * 20.0f);
}

float getMountains(glm::vec3 position, int worldSeed) 
{
	const float mountainsFactorNoiseScale = 128.0f;
	const float mountainsFactor = 
	glm::pow(1.0f - 
	glm::abs(
		PerlinNoise::get(
			position.z / mountainsFactorNoiseScale, 
			position.x / mountainsFactorNoiseScale, 
			worldSeed, 
		3) * 2.0f - 1.0f
	)/*<-abs*/, 2.0f) * glm::pow(PerlinNoise::get(
			worldSeed, 
			position.z / mountainsFactorNoiseScale / 2.0f, 
			position.x / mountainsFactorNoiseScale / 2.0f, 
	1), 4.0f);
	const float mountainsNoiseScale = 48.0f;
	const float mountainsHeight = 1.0f * mountainsNoiseScale;
	
	return PerlinNoise::get(
		worldSeed, 
		position.x / mountainsNoiseScale, 
		position.z / mountainsNoiseScale, 
		3) 
		* mountainsHeight 
		* mountainsFactor;
}

float getPlatoesMultiplier(glm::vec3 position, int worldSeed) 
{
	const float platoesSeedOffset = 0x50f7fee7;
	const float multiplierScale = 320.0f;
	const float multiplier = glm::smoothstep(
		0.48f,
		0.52f,
		PerlinNoise::get(position.x / multiplierScale, worldSeed + platoesSeedOffset, position.z / multiplierScale, 3, 0.6f));
	return multiplier;
}

float getRivers(glm::vec3 pos) 
{
	return 1.0f - glm::abs(PerlinNoise::get(pos.x / 192.0f, pos.y, pos.z / 192.0f, 3) - 0.5f) * 2.0f;
}

float getHeight(glm::vec3 position, int worldSeed) 
{
	const float heightNoiseScale = 16.0f;
	const float heightMultiplyScale = 64.0f;
	const float heightFactor = 1.2f * heightNoiseScale; 
	const float heightMult = std::pow(PerlinNoise::get(position.x / heightMultiplyScale, worldSeed + 87, position.z / heightMultiplyScale), 1.5f);
	const float height = 2.0f + PerlinNoise::get(position.x / heightNoiseScale, worldSeed, position.z / heightNoiseScale, 4, 0.6f, 1.8f) * heightFactor * (heightMult);
	const float mountains = getMountains(position, worldSeed);
	const float platoesMult = getPlatoesMultiplier(position, worldSeed);
	const float rivers = getRivers(glm::vec3(position.x, -worldSeed, position.z));
	const float riverDepth = PerlinNoise::get(position.x / 8.0f, position.z / 8.0f, position.z / 8.0f, 2) * 8.0f + 2.0f;
	const float heightWithRivers = glm::mix(height, glm::mix(
		height * (1.0f - rivers*rivers), 
		-riverDepth,
		glm::clamp(glm::pow(rivers, 0.3f) * 42.0f - 41.0f, 0.0f, 1.0f)
	), glm::clamp(4.0f / height, 0.0f, 1.0f));
	return heightWithRivers;//(height + mountains + platoesMult * 32.0f);
}


const float relaxFactor = 4.0f;
const float depthRelax = 1.0f;

float getGeneratorTerrainValueAt(glm::vec3 pos, int worldSeed) 
{
	return ((getHeight(pos, worldSeed) - pos.y) / depthRelax + getVolume(pos, worldSeed)) / relaxFactor;
}

void generateTerrain(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed) 
{
	const unsigned short shortGrassId = VoxelMaterials::getInstance().getIdByName("short_grass");
	const unsigned short slateId = VoxelMaterials::getInstance().getIdByName("slate");
	const unsigned short sandId = VoxelMaterials::getInstance().getIdByName("sand");
	const int samplesCount = 16;
	const int stepsPerSample = chunkSize.x / samplesCount;

	float samples[samplesCount + 1][samplesCount + 1][samplesCount + 1];
	float hsamples[samplesCount + 1][samplesCount + 1];

	glm::vec3 chunkPos = index * chunkSize;

	for(int i = 0; i < samplesCount + 1; i++)
	for(int j = 0; j < samplesCount + 1; j++)
	for(int k = 0; k < samplesCount + 1; k++) 
	{
		glm::vec3 localPos = glm::vec3(i,j,k) * (float)stepsPerSample;
		glm::vec3 worldPos = localPos + chunkPos;
		samples[i][j][k] = getVolume(worldPos, worldSeed);
	}


	for(int i = 0; i < samplesCount + 1; i++)
	for(int k = 0; k < samplesCount + 1; k++) 
	{
		glm::vec3 localPos = glm::vec3(i, 0.0f, k) * (float)stepsPerSample;
		glm::vec3 worldPos = localPos + chunkPos;
		hsamples[i][k] = getHeight(worldPos, worldSeed);
	}
	
	for(int i = 0; i < chunkSize.x; i++)
	for(int k = 0; k < chunkSize.z; k++) 
	{ 
		glm::ivec2 localHInd = glm::ivec2(i, k);
		glm::vec2 sampleHFract = glm::vec2(localHInd % stepsPerSample) / (float)stepsPerSample;
		glm::ivec2 sampleHInd = localHInd / stepsPerSample;
		const float h00 = hsamples[sampleHInd.x+0][sampleHInd.y+0];
		const float h10 = hsamples[sampleHInd.x+1][sampleHInd.y+0];
		const float h01 = hsamples[sampleHInd.x+0][sampleHInd.y+1];
		const float h11 = hsamples[sampleHInd.x+1][sampleHInd.y+1];
		const float height = linear2DGridInterpolation(sampleHFract, h00, h10, h01, h11);
		for(int j = 0; j < chunkSize.y; j++)
		{
			glm::ivec3 localInd = glm::ivec3(i, j, k);
			glm::vec3 worldPos = glm::vec3(localInd) + chunkPos;
			glm::vec3 sampleFract = glm::vec3(localInd % stepsPerSample) / (float)stepsPerSample;
			glm::ivec3 sampleInd = localInd / stepsPerSample;
			const float s000 = samples[sampleInd.x+0][sampleInd.y+0][sampleInd.z+0];
			const float s001 = samples[sampleInd.x+0][sampleInd.y+0][sampleInd.z+1];
			const float s010 = samples[sampleInd.x+0][sampleInd.y+1][sampleInd.z+0];
			const float s011 = samples[sampleInd.x+0][sampleInd.y+1][sampleInd.z+1];
			const float s100 = samples[sampleInd.x+1][sampleInd.y+0][sampleInd.z+0];
			const float s101 = samples[sampleInd.x+1][sampleInd.y+0][sampleInd.z+1];
			const float s110 = samples[sampleInd.x+1][sampleInd.y+1][sampleInd.z+0];
			const float s111 = samples[sampleInd.x+1][sampleInd.y+1][sampleInd.z+1];
			const float depth = height - worldPos.y;
			const float volume = glm::clamp(
				(depth / depthRelax + linear3DGridInterpolation(sampleFract, s000, s100, s010, s110, s001, s101, s011, s111)) / relaxFactor,
				 -1.0f, 1.0f);
			
			Voxel v;
			v.amount = volume * voxelAmountMax;
			v.voxelTypeId = volume > 0.0f ? (volume > 0.8f ? slateId : shortGrassId) : (0);
			chunk->voxelData.quickSetVoxel(localInd, v);
		}
	}
}