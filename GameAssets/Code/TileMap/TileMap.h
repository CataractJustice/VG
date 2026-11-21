#pragma once
#include "GameNode/Component.h"
#include "Physics/Collider.h"
#include "Kirino/VertexArrayObject.h"
#include <filesystem>
using namespace TealEngine;
class Chunk;
class TileMeshPart;
class TileMap : public Component 
{
	public:
	struct TileData 
	{
		unsigned short cubeMaterialId;
		unsigned short tileId;
		unsigned char orientation;
	};
	Chunk* parentChunk;
	private:
	static TileData defaultTile;
	protected:
	//TileData tiles[32*32*32];
	unsigned short tileIds[32*32*32];
	unsigned short cubeMaterials[32*32*32];
	int cubeTileCount = 0;
		
	public:
	TileMeshPart* meshParts[6];
	unsigned int volumeSampler;
	inline void setParentChunk(Chunk* c) { this->parentChunk = c; }
	void onAttach() override;
	void setTileAtIndex(glm::ivec3 index, const TileData& tile);
	TileData getTileAtIndex(glm::ivec3 index) const;
	TileMap::TileData tileAtWrapped(const glm::ivec3& index) const;
	unsigned short cubeAtWrapped(const glm::ivec3& index) const;
	TileMap();
	~TileMap();
	static void decomposeXYZWHL(unsigned int xyzwhl, int& x, int& y, int& z, int& w, int& h, int& l);
	static int composeXYZWHL(int x, int y, int z, int w, int h, int l);
	static unsigned short composeWHL(int w, int h, int l);
	static void decomposeWHL(unsigned short whl, int& w, int& h, int& l);
	void updateMesh();
	void uploadMesh() const;
	inline bool isEmpty() { return cubeTileCount == 0; }
	std::vector<unsigned short> compress();
	void decompress(const std::vector<unsigned short>& data);
	void save(const std::filesystem::path& path);
	void clear();
	friend class ChunkCompressor;
};