
#pragma once
#include "GameNode/Component.h"
#include "System/Clock.h"
#include "System/Comparators/ivec3Comparator.h"
#include "VoxelData.h"
#include "Math/IndexMath.h"
#include "GameNode/GameNode3D.h"
#include "Chunk.h"
#include <thread>
#include <mutex>
#include <set>
#include <map>
#include "Kirino/Material.h"
#include "System/MutexDataPair.h"
#include "System/JobUniqueQueue.h"
#include "System/Comparators/AddressComparator.h"
#include "System/JobFunel.h"
#include <queue>
#include "Player/PlayerManager.h"
#include <semaphore>
#include "ChunkBank.h"
#include "Inventory/InventoryManager.h"
#include "Entity/EntityManager.h"
#include "ChunkCompressor/ChunkCompressor.h"
using namespace TealEngine;

const float tileWorldOffset = 0.5f;

const int maxWorldThreads = 8;
const int worldIdSize = 64;
struct WorldSaveData 
{
	int seed;
	char title[64];
	char id[worldIdSize];
};

struct ChunkRequestPacket;

class World : public Component
{
private:
	unsigned int waterMaterialRenderFragPosIndex;
	unsigned int waterMaterialFluidSamplerIndex;
	unsigned int waterMaterialScreenSpaceIndex;
	unsigned int waterMaterialOffsetIndex;
	unsigned int grassMaterialOffsetIndex;
	unsigned int grassMaterialPlaneIndex;
	unsigned int authRetryCounter = 0;
	unsigned int infoRetryCounter = 0;
	unsigned int playerSaveRetryCounter = 0;
	Kirino::Material* material;
	Kirino::Material* shadowMaterial;
	Kirino::Material* tilemapMaterial;
	Kirino::Material* tilemapShadowMaterial;
	Kirino::Material* grassMaterial;
	Kirino::Material* waterMaterial;
	Kirino::Material* waterCombineMaterial;
	std::string worldName;
	glm::ivec3 chunkSize;
	std::map<glm::ivec3, Chunk*, ivec3Comparator> chunks;
	JobUniqueQueue<Chunk*, AddressComparator> meshUpdateQueue;
	JobUniqueQueue<Chunk*, AddressComparator> tileMeshUpdateQueue;
	Kirino::VertexArrayObject waterCubeVAO;

	unsigned int chunkLoadThreadNextId = 0;
	std::queue<Chunk*> chunkLoadMainQueue;
	std::queue<Chunk*> chunkLoadQueues[maxWorldThreads];
	std::mutex chunkLoadQueuesMutex[maxWorldThreads];

	unsigned int meshGenThreadNextId = 0;
	std::queue<Chunk*> chunkMeshGenMainQueue;
	std::set<Chunk*> chunkMeshGenQueues[maxWorldThreads];
	std::mutex chunkMeshGenQueuesMutex[maxWorldThreads];

	std::queue<Chunk*> meshUpdateQueueQueue;
	JobFunnel<Chunk*> meshUploadQueue;
	JobFunnel<Chunk*> tileMeshUploadQueue;
	std::set<Chunk*> existingChunks;
	std::mutex existingChunksMutex;
	glm::vec3 currentLoadPosition;
	float loadPositionUpdateDistanceSquare;
	int horizontalRenderDistance;
	int verticalRenderDistance;
	FrameBuffer waterFBO;
	Texture waterRenderTexture;
	Texture waterDepthTexture;
	Texture waterNormalTexture;

	std::mutex chunkSaveAndFreeQueueMutex;
	std::queue<Chunk*> chunkSaveAndFreeQueue;

	std::thread chunkLoadThreads[8];
	std::thread meshGenThreads[8];
	std::thread waterUpdateThread;
	std::binary_semaphore waterWorkRequest{0};
	std::binary_semaphore waterWorkFinish{1};
	std::thread saveThread;
	std::atomic<bool> alive;
	std::atomic<bool> saving;
	Chunk* uploadMeshChunk;
	Chunk* uploadTileMeshChunk;

	std::map<glm::ivec3, int, ivec3Comparator> loadMarkedChunks;
	void* chunkRequestsQueue;
	bool gotWorldInfo;
	bool storageHasBeenSetup;
	bool joinComplete = false;
	glm::ivec3 renderPosition;
	ChunkCompressor compressor;
public:
	PlayerBuilder* playerBuilder;
	PlayerManager* playerManager;
	InventoryManager* inventoryManager;
	EntityManager* entityManager;
	static WorldSaveData worldData;
	static World* currentWorld;

	Chunk* getChunkAt(glm::vec3 position);
	const Chunk* getChunkAtConst(glm::vec3 position) const;
	Voxel getVoxelAt(glm::vec3 position) const;
	TileMap::TileData getTileAt(glm::vec3 position) const;
	void setTileAt(glm::vec3 position, const TileMap::TileData& tile);
	Voxel getVoxel(glm::ivec3 index) const;
	void setVoxel(glm::ivec3 index, Voxel v); 
	void setVoxelAt(glm::vec3 position, Voxel voxel);
	float getValueAt(glm::vec3 position) const;
	glm::vec3 getNormalAt(glm::vec3 position) const;
	Chunk* getChunk(glm::ivec3 index);
	const Chunk* getChunkConst(glm::ivec3 index) const;
	void update() override;
	void onAttach() override;
	void onDestroy() override;

	void loadArea(glm::ivec3 from, glm::ivec3 to, int ticks);
	void setRenderPosition(glm::ivec3 position);

	void chunkLoadThreadMethod(unsigned int id);
	void meshGenThreadMethod(unsigned int id);
	void chunkSaveThreadMethod();

	bool chunkLoad(Chunk* chunk, ChunkCompressor& compressor);
	void chunkCreate(glm::ivec3 index, Chunk* chunk);
	bool chunkDestroy(glm::ivec3 index);
	inline void setWorldName(const std::string& name) { this->worldName = name; }
	int getLoadingChunksCount();

	bool meshRefresh(Chunk* chunk);
	bool rayCast(glm::vec3& hit, glm::vec3 dir, int maxSteps, float maxDistance, glm::ivec3& hitIndex, glm::vec3& normal);
	bool voxelCast(glm::vec3& hit, glm::vec3 dir, int maxSteps, float maxDistance, glm::ivec3& hitIndex, glm::vec3& normal, float& rayLength);
	bool cubeCast(glm::vec3& hit, glm::vec3 dir, int maxSteps, float maxDistance, glm::ivec3& hitIndex, glm::vec3& normal, float& rayLength);
	World();
	~World();

	void onClientPacket(const void* data, int size) override;
	void onHostPeerPacket(PeerInfo& peer, const void* data, int size) override;
	void onHostPeerConnect(PeerInfo& peer) override;
	//returns true if chunk was just created
	//returns false if only chunk load ticks were updated
	bool markChunkForLoad(glm::ivec3 index, int ticks, bool rendered, bool loadJob);
	void createLoadJobForChunkAtIndex(glm::ivec3 index);
	std::filesystem::path getChunkPath(glm::ivec3 index) const;
	std::filesystem::path getTilesPath(glm::vec3 index) const;
	std::filesystem::path getFluidPath(glm::vec3 index) const;
	std::filesystem::path getWorldPath() const;
	void saveAll();
	std::filesystem::path getPlayersDirectory() const;
	inline glm::ivec3 getChunkSize() { return chunkSize; }
	void waterUpdateThreadMethod();
	void render(Kirino::StateColletion& state, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) override;
	void shadowRender(Kirino::StateColletion& state, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) override;
	void transparentRender(Kirino::StateColletion& state, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) override;
	void setupWorldStorage();
	void disconnectFromTheHost();
	void clientUpdateService();
};