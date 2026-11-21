#include "Kirino/Mesh.h"
#include "Kirino/VertexArrayObject.h"
#include "GameNode/Component.h"
#include "TileMap.h"
#include "Kirino/Material.h"
using namespace TealEngine;
class TileMeshPart
{
	public:
	enum PartSide 
	{
		Left,
		Right,
		Bottom,
		Top,
		Back,
		Front
	};
	private:
	bool uploaded;
	std::vector<unsigned int> vertices;
	Kirino::VertexArrayObject vao;
	unsigned int vbo;
	unsigned int ebo;
	TileMap* tileMap;
	public:
	unsigned int quadCount;
	PartSide side;
	TileMeshPart();
	inline void setTileMap(TileMap* tilemap, PartSide side) { this->tileMap = tilemap; this->side = side; }
	void updateData();
	void uploadData();
	Kirino::VertexArrayObject& getVAO() { return vao; };
};