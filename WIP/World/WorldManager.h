#pragma once
class WorldChunkManager;
class WorldStorage;
class WorldRenderer;
class WorldNetworkManager;
class World 
{
	private:
	WorldChunkManager* chunkManager;
	WorldStorage* storage;
	WorldNetworkManager* networkManager;
	WorldRenderer* renderer;
	public:
	WorldChunkManager* getChunkManager();
	WorldStorage* getStorage();
	WorldNetworkManager* getNetworkManager();
	WorldRenderer* getRenderer();
	World();
};