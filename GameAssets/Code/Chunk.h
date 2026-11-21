#pragma once
#include "Graphics/Mesh/MeshRenderer.h"
#include "Core.h"
#include "MarchingCubesVoxelMesh.h"
#include <string>
#include "GameNode/GameNode3D.h"
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include "TileMap/TileMap.h"
#include "TileMap/FluidSim.h"
#include "TileMap/FluidSimRender.h"
class Entity;
class World;
using NeighborOffsetT = int16_t;
class Chunk
{
public:
	VoxelData voxelData;
private:
	NeighborOffsetT neighbors[26];
public:
	std::map<unsigned int, std::set<Entity*>> entities;
	TileMap tileMap;
	FluidSim fluidSim;
	GameNode3D* node;
	MarchingCubesVoxelMesh* mesh;
	std::mutex meshGenMutex;
	std::mutex tileMeshGenMutex;
	unsigned char frustumCulling	: 1;
	unsigned char terrainGenerated	: 1;
	unsigned char dirtyMesh			: 1;
	unsigned char dirtyTileMesh		: 1;
	unsigned char isFreed			: 1;
	unsigned char framesSinceMeshUpdate : 3;
	
	inline void setNeighbor(glm::ivec3 side, Chunk* chunk)
	{
		int index =
			(side.x + 1) +
			(side.y + 1) * 3 +
			(side.z + 1) * 9;
		if(index > 13) index -= 1;
		neighbors[index] = chunk ? static_cast<NeighborOffsetT>(chunk - this) : 0;
	}
	inline Chunk* getNeighbor(glm::ivec3 side)
	{
		int index =
			(side.x + 1) +
			(side.y + 1) * 3 +
			(side.z + 1) * 9;
		if(index > 13) index -= 1;
		return neighbors[index] ? this + neighbors[index] : nullptr;
	}
	
	Chunk(glm::ivec3 dimensions, glm::ivec3 index);
	~Chunk();
	Chunk();
	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;
};