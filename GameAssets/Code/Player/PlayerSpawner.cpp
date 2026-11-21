#include "Player/PlayerSpawner.h"
#include "ChunkLoader.h"
#include "World.h"
#include "GameNode/ComponentFactory.h"
#include "Player/PlayerBuilder.h"
#include "FileSystem/loadBytesFromFile.h"

PlayerSpawner::PlayerSpawner(GameNode3D* node, PlayerBuilder* playerBuilder, const std::string& uuid) : 
spawned(false),
player(node),
pb(playerBuilder),
playerUUID(uuid)
{
}

void PlayerSpawner::onAttach() 
{
	ChunkLoader* cl = new ChunkLoader();
	cl->verticalLoadDistance = 1;
	cl->horisontalLoadDistance = 1;
	this->getParent()->attachComponent(cl);
	std::filesystem::path playerPath = World::currentWorld->getPlayersDirectory() / playerUUID;
	if(std::filesystem::exists(playerPath)) 
	{
		tryGroundPlayer = false;
		loadPlayer = true;
	}
	if(loadPlayer) 
	{
		pb->loadBuffer(loadBytesFromFile(playerPath).data());
		this->getParentOfType<GameNode3D>()->getRelativeTransform().setPosition(pb->getParentOfType<GameNode3D>()->getRelativeTransform().getPosition());
	}
}

void PlayerSpawner::update() 
{
	if(spawned) return;
	bool canTrySpawn = true;
	Transform& t = this->getParentOfType<GameNode3D>()->getRelativeTransform();
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	for(int k = -1; k <= 1; k++) 
	{
		if(!canTrySpawn) break;
		glm::ivec3 chunkIndex = vec3ToIndex(t.getPosition(), World::currentWorld->getChunkSize());
		Chunk* chunk = World::currentWorld->getChunk(chunkIndex + glm::ivec3(i,j,k));
		if(chunk) 
		{
			if(chunk->voxelData.terrainGenMutex.try_lock()) 
			{
				if(chunk->terrainGenerated == false) canTrySpawn = false;
				chunk->voxelData.terrainGenMutex.unlock();
			}
			else 
			{
				canTrySpawn = false;
			}
		}
		else 
		{
			canTrySpawn = false;
		}
	}
	if(!canTrySpawn) return;
	bool canSpawn = false;
	
	
	Chunk* chunk = World::currentWorld->getChunkAt(t.getPosition());
	while(chunk != nullptr && chunk->terrainGenerated == true) 
	{
		if(tryGroundPlayer) 
		{
			if(World::currentWorld->getValueAt(t.getPosition()) > 0.0f)
			{
				t.translate(glm::vec3(0.0f, 3.0f, 0.0f));
				canSpawn = true;
				break;
			}
			t.translate(glm::vec3(0.0f, -1.0f, 0.0f)); 
		}
		else 
		{
			canSpawn = true;
		}
		if(canSpawn) break;
		chunk = World::currentWorld->getChunkAt(t.getPosition());
	}
	
	if(canSpawn) 
	{
		if(!loadPlayer) 
		{
			player->getRelativeTransform().setPosition(t.getPosition());
		}
		Core::getRoot()->addChild(player);
		this->getParent()->destroy();
		spawned = true;
	}
}