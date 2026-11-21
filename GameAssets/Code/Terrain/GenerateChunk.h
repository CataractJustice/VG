#include "glm/glm.hpp"
class Chunk;
void generateBoulders(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed);
void generateCapsuleLine(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, glm::vec4 from, glm::vec4 to, int typeID);
void generateFractalTree(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, glm::vec3 root, glm::vec3 direction, float width, float widthFactor, float lengthFactor, float directionDeviation, int stages, int treeSeed, bool rotate = false);
void generateHeight(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed);
void generateTrees(Chunk* chunk, glm::ivec3 index, glm::ivec3 realIndex, glm::ivec3 chunkSize, int worldSeed, glm::ivec3 offset);
void generateChunk(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed);
void generateCrystals(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed);
void generateBlobs(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed);
void generateOres(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, unsigned short id, unsigned short replaceId, unsigned int chunksPerSpawn, unsigned int size,  int worldSeed);
void generateMonoliths(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed);
void generateTerrain(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed);
void generateCaves(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed);
void generateFill(Chunk* chunk, unsigned short id, signed char amount);
void generateWater(Chunk* chunk, glm::ivec3 index);
float getGeneratorTerrainValueAt(glm::vec3 pos, int worldSeed);