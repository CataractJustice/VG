#include "World.h"
#include "GenerateChunk.h"
#include "Math/Noise.h"
#include "glm/glm.hpp"
#include "glm/matrix.hpp"
#include "VoxelMaterials.h"

void generateFractalTree(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, glm::vec3 root, glm::vec3 direction, float width, float widthFactor, float lengthFactor, float directionDeviation, int stages, int treeSeed, bool rotate) 
{
	const unsigned short woodId = VoxelMaterials::getInstance().getIdByName("wood");
	const unsigned short leavesId = VoxelMaterials::getInstance().getIdByName("wood");
	if(stages == 0) 
	{
		//generateCapsuleLine(chunk, index, chunkSize, glm::vec4(root, 3.0f), glm::vec4(root-direction * 0.1f, 2.0f), leavesId);
		return;
	};
	if(rotate) 
	{
		glm::vec3 randomVec = glm::vec3(Noise::get(treeSeed, stages), Noise::get(stages, treeSeed), Noise::get(-treeSeed, -stages));
		randomVec = glm::normalize(randomVec - 0.5f);
		glm::vec3 axis = glm::normalize(glm::cross(randomVec, direction));
		direction = glm::rotate(glm::mat4(1.0f), directionDeviation * Noise::get(treeSeed * 2, stages * 15, treeSeed), axis) * glm::vec4(direction, 1.0f);
	}
	glm::vec3 end = root + direction;
	generateCapsuleLine(chunk, index, chunkSize, glm::vec4(root, width), glm::vec4(end, width * widthFactor), woodId);
	generateFractalTree(chunk, index, chunkSize, glm::vec4(end, width * widthFactor), direction * lengthFactor, width*widthFactor, widthFactor, lengthFactor, directionDeviation, stages - 1, treeSeed * Noise::uintGet(treeSeed, stages, treeSeed + 1), true);
	generateFractalTree(chunk, index, chunkSize, glm::vec4(end, width * widthFactor), direction * lengthFactor, width*widthFactor, widthFactor, lengthFactor, directionDeviation, stages - 1, treeSeed * Noise::uintGet(stages, treeSeed, treeSeed - 1), true);
	generateFractalTree(chunk, index, chunkSize, glm::vec4(end, width * widthFactor), direction * lengthFactor, width*widthFactor, widthFactor, lengthFactor, directionDeviation, stages - 1, treeSeed * Noise::uintGet(stages, treeSeed*2, stages), true);
}