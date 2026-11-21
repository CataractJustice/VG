#include "BreakTool.h"
#include "System/Input.h"
#include "VoxelMaterials.h"
#include "GameNode/ComponentFactory.h"
#include "VoxelData.h"
#include "GameNode/Properties/ComponentRefProp.h"
#include "Core.h"
#include "GameNode/Properties/GameNodeProp.h"


BreakTool::BreakTool() 
{
	reach = 8.0f;
	this->addProp(new FloatProp(&reach), "reach");
}

void BreakTool::onUsePress(unsigned char usage) 
{
	if(usage != ItemUsageType::primary) return;

	const Transform& t = this->getParentOfType<GameNode3D>()->getWorldTransform();
	glm::vec3 hit = t.getPosition();
	glm::vec3 f = t.forward();
	glm::ivec3 hitIndex;
	glm::vec3 normal;
	
	bool didHit = World::currentWorld->rayCast(hit, f, 64, reach, hitIndex, normal);
	
	if(didHit) 
	{
		glm::vec3 breakPosition = hit - normal * 0.1f;
		TileMap::TileData tile = World::currentWorld->getTileAt(breakPosition);
		if(tile.cubeMaterialId != 0) 
		{
			TileMap::TileData airTile = tile;
			airTile.cubeMaterialId = 0;
			World::currentWorld->setTileAt(breakPosition, airTile);
			VoxelMaterials::getInstance().getById(tile.cubeMaterialId).breakEvent->invoke(glm::floor(breakPosition) + 0.5f, VoxelMaterials::getInstance().getById(tile.cubeMaterialId));
		}
		else 
		{	
			for(int i = 0; i <= 1; i++)
			for(int j = 0; j <= 1; j++)
			for(int k = 0; k <= 1; k++)
			{
				glm::ivec3 voxelIndex = hitIndex + glm::ivec3(i,j,k);
				Voxel voxel = World::currentWorld->getVoxel(voxelIndex);
				if(voxel.voxelTypeId != 0 && voxel.amount > 0) 
				{
					Voxel airVoxel;
					airVoxel.amount = -1;
					airVoxel.voxelTypeId = 0;
					VoxelMaterials::getInstance().getById(voxel.voxelTypeId).breakEvent->invoke(glm::floor(breakPosition) + 0.5f, VoxelMaterials::getInstance().getById(voxel.voxelTypeId));
					World::currentWorld->setVoxel(voxelIndex, airVoxel);
				}
			}
		}
	}
}

EXPORT_COMPONENT(BreakTool);
