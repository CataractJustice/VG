#pragma once
#include "VoxelData.h"
#include "Kirino/MeshBuilders/Mesh3DBuilder.h"
#include <stack>
#include "System/StaticSizeArray.h"

const int maxTrianglesPerCell = 5;
const int cellUpdateQueueMaxSize = 1024;

#pragma pack(push, 1)
struct GrassVertexInfoUnpacked
{
	//6 bits
	unsigned int rotation;
	//6 bits
	unsigned int shearF;
	//6 bits
	unsigned int shearR;
	//2 bits
	unsigned int size;
	//9 bits
	unsigned int X;
	//9 bits
	unsigned int Y;
	//9 bits
	unsigned int Z;
	//1 bit
	unsigned int variant;
};
struct GrassVertexInfoPacked
{
	unsigned int part32;
	unsigned short part16;
};
#pragma pack(pop)

class MarchingCubesVoxelMesh
{
public:
	#pragma pack(push, 1)
	struct CellData 
	{
		unsigned long long data[2];
	};
	struct Vertex 
	{
		unsigned int position, normal, textureOffsets;
	};
	struct EdgeInfo 
	{
		unsigned char offset;
		unsigned char materialId;
	};
	struct Triangle 
	{
		unsigned long long indicesAndDecorations;
		unsigned int material;
	};
	#pragma pack(pop)
private:
	VoxelData* data;
	unsigned int vbo, ebo;
	unsigned int grassVBO;
	unsigned int randomSeed;
	unsigned int grassDataSize;
	Kirino::VertexArrayObject vao;
	Kirino::VertexArrayObject grassVAO;
	Kirino::Mesh mesh;
	
	float threshold;
	unsigned int levelofdetails;
	glm::ivec3 dimensions;
	glm::vec3 offset;
	void updateCell(glm::ivec3 index);
	Vertex getCellEdgeVertex(glm::ivec3 index, unsigned int edge);
	unsigned int cellUpdateQueue[cellUpdateQueueMaxSize];
	unsigned int voxelUpdateQueue[cellUpdateQueueMaxSize];
	int cellUpdateQueueHead;
	int voxelUpdateQueueHead;
	StaticSizeArray<CellData> cells;
	StaticSizeArray<EdgeInfo> edges;
	StaticSizeArray<unsigned long long> cellUpdateMask;
	StaticSizeArray<unsigned long long> voxelUpdateMask;
	std::vector<Triangle> triangles;
	std::vector<int> triangleIndexToCellTriangleIndex;
	std::vector<MarchingCubesVoxelMesh::Vertex> compactVertices;
	
	void addTriangles(MarchingCubesVoxelMesh::Triangle* addedTriangles, int trianglesCount, CellData& cell, unsigned int cellIndex);
	void removeTriangles(CellData& cell, unsigned int cellIndex);
	void pushIndice();
	void removeIndice();

	void uploadData();
	void updateIndices();
	bool getUpdateMask(glm::ivec3 index);
	void setUpdateMask(glm::ivec3 index, bool updateSet);
	std::vector<unsigned int> indices;
	std::vector<GrassVertexInfoPacked> grassData;

	void updateVoxel(const glm::ivec3& index);

	bool allDirty = true;
	bool hasDecorations = false;

public:
	MarchingCubesVoxelMesh(VoxelData* data, float threshold, unsigned int levelofdetails, glm::vec3 offset);
	~MarchingCubesVoxelMesh();
	void updateMesh();
	void uploadMesh();
	void clear();
	void clearAll();
	inline const Kirino::Mesh& getMesh() const { return mesh; }
	void markWholeChunkDirty();
	void markVoxelForUpdate(glm::ivec3 index);
	void drawGrass();
	bool hasGrass();
};
