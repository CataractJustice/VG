#include "WorldHitscanDisplay.h"
#include "GameNode/ComponentFactory.h"
#include "System/Input.h"
#include "Core.h"
#include "World.h"
#include "Graphics/Shapes/Line3D.h"
#include "VoxelMaterials.h"

void WorldHitscanDisplay::update() 
{
	if(true) 
	{
		glm::vec3 hit = this->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition();
		glm::ivec3 hitIndex;
		glm::vec3 normal;
		bool hitSomething = World::currentWorld->rayCast(hit, this->getParentOfTypeConst<GameNode3D>()->getWorldTransform().forward(), 16, 8.0f, hitIndex, normal);
		const glm::vec4 hitColor = hitSomething ? glm::vec4(0.0f, 0.5f, 0.7f, 1.0f) : glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		const glm::vec4 hitColor2 = hitSomething ? glm::vec4(0.8f, 0.2f, 0.4f, 1.0f) : glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		
		const glm::vec4 normalColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

		Voxel v = World::currentWorld->getVoxel(hitIndex);
		VoxelMaterial vm = VoxelMaterials::getInstance().getById(v.voxelTypeId);

		float closestEmpty = 16.0f;
		glm::ivec3 indexBase = glm::ivec3(hit);
		if(hit.x < 0.0f) indexBase.x--;
		if(hit.y < 0.0f) indexBase.y--;
		if(hit.z < 0.0f) indexBase.z--;
		glm::ivec3 emptyHitIndex = indexBase;
		for(int cx = 0; cx <= 1; cx++)
		for(int cy = 0; cy <= 1; cy++)
		for(int cz = 0; cz <= 1; cz++) 
		{
			Voxel cv = World::currentWorld->getVoxel(indexBase + glm::ivec3(cx, cy, cz));
			glm::vec3 fhit = hit - glm::vec3(indexBase);
			float len = glm::length(glm::vec3(cx, cy, cz) - fhit);
			if(len < closestEmpty && cv.amount < 0) 
			{
				closestEmpty = len;
				emptyHitIndex = indexBase + glm::ivec3(cx, cy, cz);
			}
		}

		const float linewidth = 0.005f;
		if(!hitSomething) return;
		if(vm.cube) 
		{
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(0.0f, 1.0f, 0.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 1.0f, 0.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 1.0f, 1.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(hitIndex) + glm::vec3(0.0f, 1.0f, 1.0f), hitColor, linewidth));

			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 0.0f, 0.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 1.0f, 0.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 1.0f, 1.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 0.0f, 1.0f), hitColor, linewidth));

			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(0.0f, 0.0f, 1.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 0.0f, 1.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 1.0f, 1.0f), hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(0.0f, 1.0f, 1.0f), hitColor, linewidth));
		}
		else 
		{
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) - glm::vec3(1.0f, 0.0f, 0.0f) * 0.5f, glm::vec3(hitIndex) + glm::vec3(1.0f, 0.0f, 0.0f) * 0.5f, hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) - glm::vec3(0.0f, 1.0f, 0.0f) * 0.5f, glm::vec3(hitIndex) + glm::vec3(0.0f, 1.0f, 0.0f) * 0.5f, hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) - glm::vec3(0.0f, 0.0f, 1.0f) * 0.5f, glm::vec3(hitIndex) + glm::vec3(0.0f, 0.0f, 1.0f) * 0.5f, hitColor, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(emptyHitIndex) - glm::vec3(1.0f, 0.0f, 0.0f) * 0.4f, glm::vec3(emptyHitIndex) + glm::vec3(1.0f, 0.0f, 0.0f) * 0.4f, hitColor2, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(emptyHitIndex) - glm::vec3(0.0f, 1.0f, 0.0f) * 0.4f, glm::vec3(emptyHitIndex) + glm::vec3(0.0f, 1.0f, 0.0f) * 0.4f, hitColor2, linewidth));
			Core::shapesRenderer.pushShape(Line3D(glm::vec3(emptyHitIndex) - glm::vec3(0.0f, 0.0f, 1.0f) * 0.4f, glm::vec3(emptyHitIndex) + glm::vec3(0.0f, 0.0f, 1.0f) * 0.4f, hitColor2, linewidth));
		}

		Core::shapesRenderer.pushShape(Line3D(hit - glm::vec3(1.0f, 0.0f, 0.0f) * 0.1f, hit + glm::vec3(1.0f, 0.0f, 0.0f) * 0.1f, hitColor * 1.8f, linewidth));
		Core::shapesRenderer.pushShape(Line3D(hit - glm::vec3(0.0f, 1.0f, 0.0f) * 0.1f, hit + glm::vec3(0.0f, 1.0f, 0.0f) * 0.1f, hitColor * 1.8f, linewidth));
		Core::shapesRenderer.pushShape(Line3D(hit - glm::vec3(0.0f, 0.0f, 1.0f) * 0.1f, hit + glm::vec3(0.0f, 0.0f, 1.0f) * 0.1f, hitColor * 1.8f, linewidth));
		

		//Core::shapesRenderer.pushShape(Line3D(hit, hit + normal * 2.0f, normalColor));
	}
}

EXPORT_COMPONENT(WorldHitscanDisplay);