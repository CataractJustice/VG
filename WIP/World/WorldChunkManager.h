#include "libs/glm/glm.hpp"
class World;
class WorldChunkManager 
{
	private:
	void loadThreadMethod(unsigned int id);
	void saveThreadMethod(unsigned int id);
	void meshUpdateMethod(unsigned int id);
	
	public:
	WorldChunkManager(World* world);
	//Requests chunk to be loaded from disc
	void requestChunkLoadFromDisc(glm::ivec3 index);
	//Requests chunk to be created and either generated orn loaded from disc
	void requestChunkLoad(glm::ivec3 index);
	//Will unload and save chunk
	void unloadChunk(glm::ivec3 index);
	//Should be called every frame on the main thread
	void update();
	//starts worker threads
	void run();
	//Tries to stop and wait for all worker threads
	void stopAll();
};