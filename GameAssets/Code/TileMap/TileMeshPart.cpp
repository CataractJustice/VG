#include "TileMeshPart.h"
#include "Core.h"
#include <GL/glew.h>
#include "VoxelMaterials.h"

TileMeshPart::TileMeshPart() 
{
	quadCount = 0;
	uploaded = false;
	this->vao.create();
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	this->vao.vertexAttribPointer(
	Kirino::VertexArrayObject::VertexAttribPointer(
		true, 
		vbo, 
		0, 
		1,
		GL_UNSIGNED_INT,
		false, 
		sizeof(unsigned int), 
		0, 
		1)
	);
	vao.updateAttribs();
}

/*
switch (side)
{
	case PartSide::Back:	normal = {0,0,-1}; axisX = 0; axisY = 1; axisN = 2; flip = 0; break;
	case PartSide::Front:	normal = {0,0, 1}; axisX = 0; axisY = 1; axisN = 2; flip = 2; offset.z += 1.0f; break;
	case PartSide::Bottom:	normal = {0,-1,0}; axisX = 0; axisY = 2; axisN = 1; flip = 2; break;
	case PartSide::Top:		normal = {0, 1,0}; axisX = 0; axisY = 2; axisN = 1; flip = 0; offset.y += 1.0f; break;
	case PartSide::Left:	normal = {-1,0,0}; axisX = 1; axisY = 2; axisN = 0; flip = 0; break;
	case PartSide::Right:	normal = { 1,0,0}; axisX = 1; axisY = 2; axisN = 0; flip = 2; offset.x += 1.0f; break;
}
*/

//axisX = s >> 2;
//axisY = (s >> 1) + 1 - axisX;
//axisN = 2 - (s >> 1);
//flip = (((s + 1) >> 1) % 2) << 1;
//n sign = ((s % 2) << 1) - 1;


unsigned int composeQuad(int x, int y, int widthx, int widthy, int normaloffset) 
{
	return x | (y << 5) + (normaloffset << 10) + ((widthx - 1) << 15) + ((widthy - 1) << 20);
}

void TileMeshPart::uploadData() 
{
	if(vertices.size() == 0) return;
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * this->vertices.size(), this->vertices.data(), GL_STATIC_DRAW);
	this->uploaded = true;
	quadCount = vertices.size();
}

void TileMeshPart::updateData()
{
	vertices.clear();
	if(tileMap->isEmpty()) 
	{
		return;
	}
	constexpr int CHUNK_SIZE = 32;
	constexpr int MASK_SIZE  = CHUNK_SIZE * CHUNK_SIZE;

	// Each element represents one row of 32 bits.
	// A set bit means: face already merged into a quad.
	std::array<uint32_t, MASK_SIZE> mask{}; // zero-initialized

	// ---------------------------------------------------------------------
	// Configure axis mapping and normal direction
	// ---------------------------------------------------------------------
	glm::ivec3 normal(0);
	int axisX = 0;
	int axisY = 0;
	int axisN = 0;

	switch (side)
	{
		case PartSide::Back:   normal = {0,0,-1}; axisX = 0; axisY = 1; axisN = 2; break;
		case PartSide::Front:  normal = {0,0, 1}; axisX = 0; axisY = 1; axisN = 2; break;
		case PartSide::Left:   normal = {-1,0,0}; axisX = 1; axisY = 2; axisN = 0; break;
		case PartSide::Right:  normal = { 1,0,0}; axisX = 1; axisY = 2; axisN = 0; break;
		case PartSide::Top:	normal = {0, 1,0}; axisX = 0; axisY = 2; axisN = 1; break;
		case PartSide::Bottom: normal = {0,-1,0}; axisX = 0; axisY = 2; axisN = 1; break;
	}

	// ---------------------------------------------------------------------
	// Greedy meshing per slice
	// ---------------------------------------------------------------------
	for (int slice = 0; slice < CHUNK_SIZE; ++slice)
	{
		for (int y = 0; y < CHUNK_SIZE; ++y)
		{
			const int rowIndex = slice * CHUNK_SIZE + y;

			for (int x = 0; x < CHUNK_SIZE; ++x)
			{
				// Already merged?
				if (mask[rowIndex] & (1u << x))
					continue;

				// Build base voxel index
				glm::ivec3 baseIndex;
				baseIndex[axisX] = x;
				baseIndex[axisY] = y;
				baseIndex[axisN] = slice;

				const unsigned short tile = tileMap->cubeAtWrapped(baseIndex);
				if (!tile)
					continue;

				const unsigned short neighbor =
					tileMap->cubeAtWrapped(baseIndex + normal);

				if (neighbor)
					continue;

				// ---------------------------------------------------------
				// Start quad
				// ---------------------------------------------------------
				int quadWidth  = 1;
				int quadHeight = 1;

				mask[rowIndex] |= (1u << x);

				// ---------------------------------------------------------
				// Expand in X direction
				// ---------------------------------------------------------
				while (x + quadWidth < CHUNK_SIZE)
				{
					const int testX = x + quadWidth;

					if (mask[rowIndex] & (1u << testX))
						break;

					glm::ivec3 testIndex = baseIndex;
					testIndex[axisX] = testX;

					const unsigned short testTile =
						tileMap->cubeAtWrapped(testIndex);

					const unsigned short testNeighbor =
						tileMap->cubeAtWrapped(testIndex + normal);

					if (!testTile || testNeighbor)
						break;

					mask[rowIndex] |= (1u << testX);
					++quadWidth;
				}

				// ---------------------------------------------------------
				// Expand in Y direction
				// ---------------------------------------------------------
				while (y + quadHeight < CHUNK_SIZE)
				{
					const int testY = y + quadHeight;
					const int testRow = slice * CHUNK_SIZE + testY;

					bool canExpand = true;

					for (int dx = 0; dx < quadWidth; ++dx)
					{
						glm::ivec3 testIndex = baseIndex;
						testIndex[axisX] = x + dx;
						testIndex[axisY] = testY;

						const unsigned short testTile =
							tileMap->cubeAtWrapped(testIndex);

						const unsigned short testNeighbor =
							tileMap->cubeAtWrapped(testIndex + normal);

						if (!testTile || testNeighbor)
						{
							canExpand = false;
							break;
						}
					}

					if (!canExpand)
						break;

					// Mark entire row segment
					mask[testRow] |= ((1u << quadWidth) - 1u) << x;
					++quadHeight;
				}

				// ---------------------------------------------------------
				// Emit quad
				// ---------------------------------------------------------
				unsigned int q = composeQuad(x, y, quadWidth, quadHeight, slice);
				vertices.push_back(q);
			}
		}
	}
}