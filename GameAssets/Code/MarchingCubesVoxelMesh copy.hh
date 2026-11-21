#pragma once
#include "VoxelData.h"
#include "Kirino/MeshBuilders/Mesh3DBuilder.h"
#include <stack>
#include "System/StaticSizeArray.h"

const int maxTrianglesPerCell = 5;
const int cellUpdateQueueMaxSize = 1024;

class MarchingCubesVoxelMesh
{
private:
	VoxelData* data;
	unsigned int vbo, ebo;
	Kirino::VertexArrayObject vao;
	Kirino::Mesh mesh;
	bool hasDecorations;
	std::vector<glm::vec3> decVertices;
	std::vector<glm::vec3> decNormals;
	std::vector<glm::vec2> decUVs;
	std::vector<unsigned int> decIndices;
	float threshold;
	unsigned int levelofdetails;
	glm::ivec3 dimensions;
	glm::vec3 offset;
	struct CellData 
	{
		unsigned int decorationIndicesBase;
		unsigned int decorationIndicesCount;
		int triangleIndexes[maxTrianglesPerCell];
		unsigned char tableIndex;
	};
	struct Vertex 
	{
		unsigned int position, normal, textureOffsets;
	};
	#pragma pack(push, 1)
	struct Triangle 
	{
		unsigned int indices[3];
	};
	#pragma pack(pop)
	void updateCell(glm::ivec3 index);
	Vertex getCellEdgeVertex(glm::ivec3 index, unsigned int edge);
	unsigned int* cellUpdateMask;
	unsigned int cellUpdateMaskSize;
	unsigned int cellUpdateQueue[cellUpdateQueueMaxSize];
	int cellUpdateQueueHead;
	StaticSizeArray<CellData> cells;
	StaticSizeArray<Vertex> edgeVertices;
	StaticSizeArray<int> edgeIndexToCompactVerticeMap;
	std::vector<Triangle> triangles;
	std::vector<int> triangleIndexToCellTriangleIndex;
	std::vector<int> compactVerticeToEdgeIndexMap;
	std::vector<MarchingCubesVoxelMesh::Vertex> compactVertices;
	int updateVertex(unsigned int edgeIndex, bool active);
	void addTriangle(MarchingCubesVoxelMesh::Triangle triangle, CellData& cell, unsigned int cellIndex);
	void removeTriangles(CellData& cell);
	void pushIndice();
	void removeIndice();

	void uploadData();
	void updateIndices();
	bool getUpdateMask(glm::ivec3 index);
	void setUpdateMask(glm::ivec3 index, bool updateSet);
	std::vector<unsigned int> indices;
	//6 bits x edge
	//6 bits y edge
	//2 bits normal
	//18 bits position

	bool allDirty = true;

public:
	Kirino::Mesh3DBuilder decorations;
	MarchingCubesVoxelMesh(VoxelData* data, float threshold, unsigned int levelofdetails, glm::vec3 offset);
	~MarchingCubesVoxelMesh();
	void updateMesh();
	void uploadMesh();
	void clear();
	inline const Kirino::Mesh& getMesh() const { return mesh; }
	void markWholeChunkDirty();
	void markVoxelForUpdate(glm::ivec3 index);
};
