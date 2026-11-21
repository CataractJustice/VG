#include "Chunk.h"
Chunk::Chunk(glm::ivec3 dimensions, glm::ivec3 index) : voxelData(dimensions, index, ScalarFieldWrapping::NEIGHBORED_CHUNK)
{
	//fluidSim = new FluidSim();
	fluidSim.parentChunk = this;
	//tileMap = new TileMap();
	meshGenMutex.lock();
	node = new GameNode3D();
	mesh = new MarchingCubesVoxelMesh(&voxelData, 0.0f, 1, glm::vec3(0.0f));
	node->attachComponent(&tileMap);
	terrainGenerated = false;
	voxelData.chunk = this;
	dirtyMesh = false;
	dirtyTileMesh = false;
	memset(&neighbors, 0, sizeof(neighbors));
	meshGenMutex.unlock();
	tileMap.setParentChunk(this);
	isFreed = true;
	framesSinceMeshUpdate = 0;
};

Chunk::~Chunk() 
{
}