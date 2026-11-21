#include "getEntitiesInRange.h"
#include "Entity/Entity.h"
#include "Math/IndexMath.h"
#include "World.h"

constexpr float len2(const glm::vec3& vec) 
{
	return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
} 

constexpr float dist2(const glm::vec3& a, const glm::vec3& b) 
{
	const float dx = a.x-b.x;
	const float dy = a.y-b.y;
	const float dz = a.z-b.z;
	return dx*dx+dy*dy+dz*dz;
}

Entity* getClosestEntityInRange(std::initializer_list<unsigned int> types, glm::vec3 position, float range) 
{
	Entity* closest = nullptr;
	const float chunkSize = 32.0f;
	float range2 = range*range;
	float closestDistance2 = range2 + 1488.0f;
	glm::ivec3 chunkRangeStart = 
		vec3ToIndex(
			position - range, 
			glm::vec3(chunkSize)
		);
	glm::ivec3 chunkRangeEnd = 
		vec3ToIndex(
			position + range, 
			glm::vec3(chunkSize)
		);
	for(int x = chunkRangeStart.x; x <= chunkRangeEnd.x; x++)
	for(int y = chunkRangeStart.y; y <= chunkRangeEnd.y; y++)
	for(int z = chunkRangeStart.z; z <= chunkRangeEnd.z; z++) 
	{
		//Sphere range to chunk bounds check
		const glm::vec3 chunkCenter = glm::vec3(x, y, z) * chunkSize + chunkSize * 0.5f;
		const glm::vec3 local = position - chunkCenter;
		const glm::vec3 halfSize = glm::vec3(chunkSize * 0.5f);
		const glm::vec3 closestCubePoint = glm::clamp(local, -halfSize, halfSize);
		const glm::vec3 delta = local - closestCubePoint;
		if (len2(delta) > range2)
		{
			/*
			if the sphere moves along a cube-aligned axis (z in this case) and its distance from the cube's center increases, 
			then it cannot start intersecting the cube if it wasn't intersecting before.
			*/
			if(z*chunkSize > position.z) 
			{
				break;
			}
			continue;
		}
		/*
			Invariants:
			Entity can exist only in one chunk
			Entity has only one type
		*/
		const Chunk* chunk = World::currentWorld->getChunkConst(glm::ivec3(x,y,z));
		if(!chunk) continue;
		for(unsigned int type : types) 
		{
			auto any = chunk->entities.find(type);
			if(any == chunk->entities.cend()) continue;
			for(Entity* e : any->second) 
			{
				const float eDst2 = dist2(
					position,
					e->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition()
				);
				if(eDst2 < closestDistance2) 
				{
					closest = e;
					closestDistance2 = eDst2;
					range2 = eDst2;
					range = glm::sqrt(eDst2);
					chunkRangeStart = 
					vec3ToIndex(
						position - range, 
						glm::vec3(chunkSize)
					);
					x = std::max(chunkRangeStart.x, x);
					y = std::max(chunkRangeStart.y, y);
					z = std::max(chunkRangeStart.z, z);
					chunkRangeEnd = 
					vec3ToIndex(
						position + range, 
						glm::vec3(chunkSize)
					);
				}
			}
		}
	}
	return closest;
}