#pragma once
#include "Math/IndexConvert.h"
#include "System/Debug.h"
#include "libs/glm/glm.hpp"
#include <filesystem>
#include <mutex>
#include <vector>
#include "GameNode/GameNode.h"
#include "Physics/Collider.h"
using namespace TealEngine;
class Chunk;

const int voxelAmountMax = 124;
const int voxelAmountMin = -124;
const float voxelAmountScale = 124.0f;

#pragma pack(push, 1)
struct Voxel 
{
public:
	signed char amount;
	unsigned short voxelTypeId;
};
enum class ScalarFieldWrapping
{
	ERROR,
	DEFAULT_VALUE,
	REPEAT,
	CLAMP_TO_BORDER,
	CLAMP,
	NEIGHBORED_CHUNK
};

enum class VoxelModifyMethod
{
	SET_VOXEL = 0,
	ADD_VOXEL = 1,
	BREAK_VOXEL = 1 << 1,
	KEEP_TYPE = 1 << 2,
	KEEP_AMOUNT = 1 << 3,
	IF_EMPTY = 1 << 4,
	IF_NOT_EMPTY = 1 << 5,
	IF_SAME_TYPE = 1 << 6,
	IF_NOT_SAME_TYPE = 1 << 7,
	IF_EXPOSED = 1 << 8,
	IF_NOT_EXPOSED = 1 << 9
};

const unsigned int chunkHardDimensions = 32;

class VoxelData
{
public:
private:
	unsigned short voxelTypeIds[chunkHardDimensions*chunkHardDimensions*chunkHardDimensions];
	signed char amounts[chunkHardDimensions*chunkHardDimensions*chunkHardDimensions];
	int xyLineMass[chunkHardDimensions*chunkHardDimensions];
	int xSliceMass[chunkHardDimensions];
	const VoxelData* neighbors[27];
	glm::ivec3 dimensions;
	glm::ivec3 index;
	unsigned long long lastModify;
	int mass;
	unsigned int datalength;
	unsigned int neighborsCount;
	bool generated;
	bool voxelNeighborCounting;
public:
	inline void setVoxelNeighborCounting(bool val) { this->voxelNeighborCounting = val; }
	static Voxel defaultValue;
	std::mutex terrainGenMutex;
	Chunk* chunk;

	VoxelData(glm::ivec3 dimensions, glm::ivec3 index = glm::ivec3(0), ScalarFieldWrapping wrapMode = ScalarFieldWrapping::DEFAULT_VALUE);
	~VoxelData();
	glm::ivec3 getDimensions();
	glm::ivec3 getIndex();
	unsigned int getDataLength();
	
	VoxelData* getNeighbor(glm::ivec3 side);
	const VoxelData* getNeighborConst(glm::ivec3 side) const;
	void setNeighbor(glm::ivec3 side, const VoxelData* data);
	unsigned int getNeighborCount() const;

	const Voxel getVoxelConst(glm::ivec3 position) const;
	signed char getVoxelValue(glm::ivec3 position) const;
	signed char getVoxelValue(int x, int y, int z) const;
	const Voxel getVoxelWrappedConst(glm::ivec3 position) const;
	signed char getVoxelValueWrapped(glm::ivec3 position) const;
	signed char getVoxelValueWrapped(int x, int y, int z) const;

	float getInterpolatedFieldValue(glm::vec3 position) const;
	glm::vec3 getNormal(glm::vec3 position) const;
	void quickSetVoxel(glm::ivec3 position, Voxel voxel);
	//bool isXLayerEmpty(unsigned int x) const;
	bool isChunkEmpty() const;
	bool isChunkFilled() const;

	inline int getXSliceMass(int x) const { return xSliceMass[x]; }
	inline int getXYLineMass(int x, int y) const { return xyLineMass[y + x * dimensions.y]; }
	inline int getMass() const { return mass; }
	inline bool isGenerated() const { return generated; }
	inline void setGenerated() { generated = true; }

	//void rebuildCompressed();
	static bool isEmpty(Voxel voxel);
	static bool isVisible(Voxel voxel);
	void save(const std::filesystem::path& path);
	bool load(const std::filesystem::path& path);
	void flattenData();
	//Returns ULONG_MAX if failed to get last modify (chunk doesn't exist, read error or chunk corrupted)
	static unsigned long long getChunkFileLastModify(const std::filesystem::path& path);
	inline unsigned long long getLastModify() const { return lastModify; };
	//
	inline void setIndex(glm::ivec3 index) { this->index = index; }
	std::vector<unsigned short> Compress();
	void Decompress(const unsigned short* data);
	std::vector<char> saveData();
	friend class ChunkCompressor;
};

#pragma pack(pop)