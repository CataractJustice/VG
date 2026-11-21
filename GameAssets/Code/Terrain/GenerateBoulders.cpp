#include "GenerateChunk.h"
#include "World.h"
#include "Math/Noise.h"
#include "VoxelMaterials.h"
void generateBoulders(Chunk* chunk, glm::ivec3 index, glm::ivec3 chunkSize, int worldSeed) 
{
	const unsigned short cobbleId = VoxelMaterials::getInstance().getIdByName("cobble");
	if(Noise::uintGet(index.x, worldSeed, index.z) % 12 == 0) 
	{
		int r = Noise::uintGet(index.z, index.x, worldSeed) % 3 + 2;
		int x = (Noise::uintGet(index.z * 2, worldSeed, index.x) % (chunkSize.x - r * 2)) + r;
		int z = (Noise::uintGet(index.z, worldSeed, index.x * 2) % (chunkSize.z - r * 2)) + r;
		int y = chunkSize.y - 1 - r;
		const int ymax = y;
		if(chunk->voxelData.getVoxelValue(glm::ivec3(x, y, z)) > 0) return;
		else
		{
			while(y >= 0 && chunk->voxelData.getVoxelValue(glm::ivec3(x, y, z)) <= 0) y--;
			if(y < 0) return;
			y += r / 2;
			y = glm::min(y, ymax);
			for(int i = -r; i <= r; i++) 
			{
				int vx = x + i;
				for(int j = -r; j <= r; j++) 
				{
					int vy = y + j;
					if(vy < 0) continue;
					for(int k = -r; k <= r; k++) 
					{
						int vz = z + k;
						Voxel v = chunk->voxelData.getVoxelConst(glm::ivec3(vx, vy, vz));
						signed char val = (1.0f - glm::length(glm::vec3(i,j,k)) / float(r)) * voxelAmountScale;
						val -= Noise::uintGet(index * chunkSize + glm::ivec3(vx, vy, vz)) % (voxelAmountMax / 4);
						if(val > voxelAmountMax) val = voxelAmountMax;
						if(val < voxelAmountMin) val = voxelAmountMin;
						if(v.amount < val) 
						{
							v.amount = val;
							v.voxelTypeId = cobbleId;
							chunk->voxelData.quickSetVoxel(glm::ivec3(vx, vy, vz), v);
						}
					}	
				}		
			}
		}
	}
}