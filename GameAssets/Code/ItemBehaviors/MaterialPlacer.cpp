#include "MaterialPlacer.h"
#include "System/Input.h"
#include "VoxelMaterials.h"
#include "GameNode/ComponentFactory.h"
#include "VoxelData.h"
#include "World.h"

MaterialPlacer::MaterialPlacer() 
{
	this->addProp(new FloatProp(&speed), "speed");
	this->addProp(new FloatProp(&reach), "reach");
	this->addProp(new FloatProp(&radius), "radius");
}
/*
const unsigned int cubePacketMagic = 0xccbe1ace;
#pragma pack(push, 1)
struct CubePlacePacket 
{
	unsigned int magic = cubePacketMagic;
	int x;
	int y;
	int z;
};
struct ClientPlaceCubeBuffer 
{
	PlayerUseItemPacket header;
	CubePlacePacket data;
};	
#pragma pack(pop)



void MaterialPlacer::processPacketHost(const void* data, unsigned int size) 
{
	if(size != sizeof(CubePlacePacket)) return;
	CubePlacePacket& packet = *((CubePlacePacket*)data);
	if(packet.magic != cubePacketMagic) return;
	const Transform& wt = this->getParentOfType<GameNode3D>()->getWorldTransform();
	if(glm::distance2(wt.getPosition(), glm::vec3(packet.x, packet.y, packet.z) + 0.5f) < (this->reach * this->reach)) 
	{
		InventoryItem& ii = *item->itemPtr;
		ItemTemplateId itd = ii.getId();
		if(ii.amount <= 0) return;
		if(!itd.starts_with("material.")) return;
		TileMap::TileData tile;
		tile.cubeMaterialId = VoxelMaterials::getInstance().getIdByName(itd.substr(9));
		tile.orientation = 0;
		tile.tileId = 0;
		World::currentWorld->setTileAt(glm::vec3(packet.x, packet.y, packet.z) + 0.1f, tile);
		ii.amount--;
	}
}
*/
void MaterialPlacer::onUsePress(unsigned char usage) 
{
	if(Core::client.isConnected()) return;
	if(usage != ItemUsageType::secondary) return;
	const Transform& wt = this->getParentOfType<GameNode3D>()->getWorldTransform();
	glm::vec3 hit = wt.getPosition();
	glm::vec3 normal;
	glm::ivec3 index;
	if(World::currentWorld->rayCast(hit, wt.forward(), 64, 8.0f, index, normal)) 
	{
		InventoryItem& ii = *item->itemPtr;
		ItemTemplateId itd = ii.getId();
		if(ii.amount <= 0) return;
		if(!itd.starts_with("material.")) return;
		TileMap::TileData tile;
		tile.cubeMaterialId = VoxelMaterials::getInstance().getIdByName(itd.substr(9));
		tile.orientation = 0;
		tile.tileId = 0;
		World::currentWorld->setTileAt(hit + normal * 0.1f, tile);
		ii.amount--;
		item->slotPtr->sync();
	}
}

EXPORT_COMPONENT(MaterialPlacer);
