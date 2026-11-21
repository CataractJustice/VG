#include "World.h"
#include "GameNode/ComponentFactory.h"
#include "Math/PerlinNoise.h"
#include "Math/ValueNoiseInterpolator.h"
#include "Terrain/GenerateChunk.h"
#include <fstream>
#include "VoxelMaterials.h"
#include "Math/linearSolve.h"
#include "Player/PlayerBuilder.h"
#include "Player/PlayerSpawner.h"
#include "Player/PlayerManager.h"
#include "FileSystem/loadBytesFromFile.h"
#include "Graphics/Shapes/Line3D.h"
#include "TileMap/TileMeshPart.h"
#include "Graphics/Renderer/RenderUtil.h"
#include "Graphics/Mesh/MeshUtil.h"
#include "System/Logger.h"
#include "System/Random.h"

bool logChunkPackets = false;

ChunkBank chunksAllocator;

const std::filesystem::path worldsDirectory = "./data/worlds/";
const char chunksDirectory[] = "c";
const char saveDataFile[] = "save";
World* World::currentWorld = nullptr;
WorldSaveData World::worldData;

const int threadsUsed = 8;
const unsigned int renderDistance = 2;

const float terrainUpdateRange = 320.0f;

typedef std::queue<std::pair<PeerInfo, ChunkRequestPacket>> chunkReqQueueT;


enum class WorldPacketType 
{
	RequestChunk,
	LoadFromCache,
	ChunkData,
	ChunkUnload,
	WorldInfo,
	WorldInfoRequest,
	SetVoxel,
	SetCubeTile
};
#pragma pack(push, 1)
struct WorldPacketHeader 
{
	unsigned char packetType;
	WorldPacketHeader(WorldPacketType pt) 
	{
		packetType = (unsigned char)pt; 
	}
};

struct ChunkRequestPacket
{
	WorldPacketHeader header;
	int x;
	int y;
	int z;
	unsigned long long lastModify;
	ChunkRequestPacket(int x, int y, int z, unsigned long long lm) : 
	x(x), 
	y(y), 
	z(z), 
	lastModify(lm), 
	header(WorldPacketType::RequestChunk) 
	{};
	ChunkRequestPacket() : header(WorldPacketType::RequestChunk) {};
};

struct LoadFromCachePacket
{
	WorldPacketHeader header;
	int x;
	int y;
	int z;
	LoadFromCachePacket(int x, int y, int z) : x(x), y(y), z(z), header(WorldPacketType::LoadFromCache) {};
};

struct ChunkDataPacket 
{
	WorldPacketHeader header;
	int x;
	int y;
	int z;
	unsigned int voxelSize;
	unsigned int tilesSize;
	unsigned int waterSize;
	ChunkDataPacket(int x, int y, int z, unsigned int vSize, unsigned int tSize, unsigned int wSize) : 
	x(x), 
	y(y), 
	z(z), 
	voxelSize(vSize), 
	tilesSize(tSize), 
	waterSize(wSize), 
	header(WorldPacketType::ChunkData) 
	{

	};
};

struct WorldInfoPacket
{
	WorldPacketHeader header;
	unsigned char id[worldIdSize];
	WorldInfoPacket() : header(WorldPacketType::WorldInfo) {};
};

struct WorldInfoRequestPacket 
{
	WorldPacketHeader header;
	WorldInfoRequestPacket() : header(WorldPacketType::WorldInfoRequest) {};
};

struct SetVoxelPacket
{	
	WorldPacketHeader header;
	int x;
	int y;
	int z;
	char amount;
	unsigned short id;
	SetVoxelPacket() : header(WorldPacketType::SetVoxel) {};
};

struct SetCubeTilePacket 
{
	WorldPacketHeader header;
	int x;
	int y;
	int z;
	unsigned short cubeId;
	SetCubeTilePacket() : header(WorldPacketType::SetCubeTile) {};
};
#pragma pack(pop)
World::World()
{
	uploadTileMeshChunk = nullptr;
	chunkSize = glm::ivec3(32, 32, 32);
	currentLoadPosition = glm::vec3(0.0f);
	loadPositionUpdateDistanceSquare = 32.0f;
	verticalRenderDistance = 3;
	horizontalRenderDistance = 5;
	this->addProp(new StringKeyMapValuePointerProp<Kirino::Material>(&material, &Core::materialsManager.getLoadedMap()), "Material");
	this->addProp(new StringKeyMapValuePointerProp<Kirino::Material>(&shadowMaterial, &Core::materialsManager.getLoadedMap()), "Shadow material");
	currentWorld = this;
	uploadMeshChunk = nullptr;
	this->worldName = World::worldData.title;
	randomBytes(World::worldData.id, worldIdSize);
	const char uuidChars[] = "abcdefghijklmnopqrstuvwxyz0123456789.-_";
	for(int i = 0; i < worldIdSize; i++) 
	{
		worldData.id[i] = uuidChars[worldData.id[i] % (sizeof(uuidChars) - 1)];
	}
	chunkRequestsQueue = new chunkReqQueueT;
	playerManager = new PlayerManager();
	inventoryManager = new InventoryManager();
	entityManager = new EntityManager();
	storageHasBeenSetup = false;
	gotWorldInfo = false;
}

World::~World() 
{
	
}

void World::onAttach() 
{
	const float waterCubeVertices[108] = {
		//0
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,

		//1
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,

		//2
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,

		//3
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,

		//4
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		//5
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 1.0f,
		
		1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f
	};
	unsigned int waterCubeVBO;
	waterCubeVAO.create();
	glBindVertexArray(waterCubeVAO);
	glGenBuffers(1, &waterCubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, waterCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 108, waterCubeVertices, GL_STATIC_DRAW);
	waterCubeVAO.vertexAttribPointer(Kirino::VertexArrayObject::VertexAttribPointer(true, waterCubeVBO, 0, 1, GL_FLOAT, false, sizeof(float) * 3, 0, 0));
	waterCubeVAO.updateAttribs();
	waterRenderTexture = Texture(GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT);
	waterNormalTexture = Texture(GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT);
	
	waterRenderTexture.setParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	waterRenderTexture.setParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	waterNormalTexture.setParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	waterNormalTexture.setParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	waterDepthTexture = Texture(GL_TEXTURE_2D, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
	if(this->material) 
	{
		unsigned int texture = VoxelMaterials::getInstance().materialsTexture.id();
		glm::vec2 atlasScale = glm::vec2(1.0f, (float)VoxelMaterials::getInstance().getMaterials().size() * 6 * 2);
		this->material->setTexture("textureImage", &texture, 1, 0);
		this->material->setUniform("atlasScale", &atlasScale, sizeof(float) * 2, 0);
	}

	this->tilemapMaterial = const_cast<Kirino::Material*>(&Core::materialsManager.get("TileMap"));
	this->tilemapShadowMaterial = const_cast<Kirino::Material*>(&Core::materialsManager.get("TileMapDepth"));
	unsigned int texture = VoxelMaterials::getInstance().materialsTexture.id();
	float atlasScaleInv = 1.0f / ((float)VoxelMaterials::getInstance().getMaterials().size() * 6.0f * 2.0f);
	this->tilemapMaterial->setTexture("textureImage", &texture, 1, 0);
	this->tilemapMaterial->setUniform("atlasScaleInv", &atlasScaleInv, sizeof(float), 0);
	this->waterMaterial = const_cast<Kirino::Material*>(&Core::materialsManager.get("Water"));
	this->grassMaterial = const_cast<Kirino::Material*>(&Core::materialsManager.get("Grass"));
	this->waterCombineMaterial = const_cast<Kirino::Material*>(&Core::materialsManager.get("WaterCombine"));
	unsigned int waterAnimTexture = Core::textureManager.get("Water256.png").id();
	waterCombineMaterial->setTexture("WaterAnimation", &waterAnimTexture);
	waterRenderTexture.create(640, 480);
	waterDepthTexture.create(640, 480);
	waterNormalTexture.create(640,480);
	waterFBO.attachTexture(waterRenderTexture.id(), 0);
	waterFBO.attachTexture(waterNormalTexture.id(), 1);
	waterFBO.attachDepthTexture(waterDepthTexture.id());
	waterMaterialRenderFragPosIndex = waterMaterial->getTextureIndex("renderFragPos");
	waterMaterialFluidSamplerIndex = waterMaterial->getTextureIndex("fluidSampler");
	waterMaterialOffsetIndex = waterMaterial->getUniformIndex("offset");
	waterMaterialScreenSpaceIndex = waterMaterial->getUniformIndex("screenspace");

	grassMaterialOffsetIndex = grassMaterial->getUniformIndex("offset");
	grassMaterialPlaneIndex = grassMaterial->getUniformIndex("plane");

	if(!alive.load()) 
	{
		alive.store(true);
		for(unsigned int i = 0; i < threadsUsed; i++) 
		{
			chunkLoadThreads[i] = std::thread(&World::chunkLoadThreadMethod, this, i);
			meshGenThreads[i] = std::thread(&World::meshGenThreadMethod, this, i);
		}
		saveThread = std::thread(&World::chunkSaveThreadMethod, this);
		waterUpdateThread = std::thread(&World::waterUpdateThreadMethod, this);
	}
	
	playerManager->setNetworkId({Core::networkDictionary.getNetIdOf("PlayerManager"), 0});
	inventoryManager->setNetworkId({Core::networkDictionary.getNetIdOf("InventoryManager"), 0});
	entityManager->setNetworkId({Core::networkDictionary.getNetIdOf("EntityManager"), 0});
	this->getParent()->attachComponent(playerManager);
	this->getParent()->attachComponent(inventoryManager);
	this->getParent()->attachComponent(entityManager);

	/*
	GameNode3D* playerSpawnerNode = new GameNode3D();
	PlayerSpawner* playerSpawner = new PlayerSpawner();
	playerSpawner->playerUUID = playerManager->getUUID();
	playerSpawner->playerType = PlayerBuilder::PlayerType::ControlledPlayer;
	Core::getRoot()->addChild(playerSpawnerNode);
	playerSpawnerNode->attachComponent(playerSpawner);
	playerSpawnerNode->getRelativeTransform().setPosition(glm::vec3(0.0f, 256.0f, 0.0f));
	*/
}

void World::onDestroy() 
{
	saveAll();
	alive.store(false, std::memory_order::relaxed);
	waterWorkRequest.release();
	for(int i = 0; i < threadsUsed; i++) 
	{
		if(chunkLoadThreads[i].joinable()) chunkLoadThreads[i].join();// = std::thread(&World::chunkLoadThreadMethod, this);
		if(meshGenThreads[i].joinable()) meshGenThreads[i].join();// = std::thread(&World::meshGenThreadMethod, this);
	}
	if(saveThread.joinable()) saveThread.join();
	if(waterUpdateThread.joinable()) waterUpdateThread.join();
	delete (chunkReqQueueT*)chunkRequestsQueue;
}

const Chunk* World::getChunkAtConst(glm::vec3 position) const
{
	auto it = chunks.find(vec3ToIndex(position, chunkSize));
	if(it == chunks.cend()) 
	{
		return nullptr;
	}
	return it->second;
}

Chunk* World::getChunkAt(glm::vec3 position) 
{
	return const_cast<Chunk*>(getChunkAtConst(position));
}

Voxel World::getVoxelAt(glm::vec3 position) const
{
	if(isnan(position.x) || isnan(position.y) || isnan(position.z)) 
	{
		return {voxelAmountMin, 0};
	}
	const Chunk* chunk = getChunkAtConst(position);
	if(chunk) 
	{
		return chunk->voxelData.getVoxelConst(ivec3RealMod(vec3ToIndex(position), chunkSize));
	}
	else 
	{	
		return {voxelAmountMin, 0};
	}
}

TileMap::TileData World::getTileAt(glm::vec3 position) const
{
	position += tileWorldOffset;
	if(isnan(position.x) || isnan(position.y) || isnan(position.z)) 
	{
		return {0,0,0};
	}
	const Chunk* chunk = getChunkAtConst(position);
	if(chunk) 
	{
		return chunk->tileMap.getTileAtIndex(ivec3RealMod(vec3ToIndex(position), chunkSize));
	}
	else 
	{	
		return {0,0,0};
	}
}

Voxel World::getVoxel(glm::ivec3 index) const 
{
	glm::ivec3 chunkIndex = ivec3ToIndex(index, chunkSize);
	if(chunks.find(chunkIndex) == chunks.cend()) return {}; 
	return chunks.at(chunkIndex)->voxelData.getVoxelConst(ivec3RealMod(index, chunkSize));
}

void World:: setVoxel(glm::ivec3 index, Voxel v) 
{
	glm::ivec3 chunkIndex = ivec3ToIndex(index, chunkSize);
	glm::ivec3 localIndex = ivec3RealMod(index, chunkSize);
	auto it = chunks.find(chunkIndex);
	if(it == chunks.cend()) return; 
	it->second->voxelData.quickSetVoxel(localIndex, v);
	it->second->dirtyMesh = true;
	Chunk* chunk = it->second;
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	for(int k = -1; k <= 1; k++) 
	{
		Chunk* neighbor = chunk->getNeighbor(glm::ivec3(i,j,k));
		if(neighbor == nullptr) continue;
		if(
			localIndex.x == 0 && i == -1 ||
			localIndex.y == 0 && j == -1 ||
			localIndex.z == 0 && k == -1 ||
			localIndex.x == (chunkSize.x - 1) && i == 1 ||
			localIndex.y == (chunkSize.y - 1) && j == 1 ||
			localIndex.z == (chunkSize.z - 1) && k == 1
		) 
		{
			neighbor->dirtyMesh = true;
		}
	}

	it->second->mesh->markVoxelForUpdate(localIndex);
	for(int i = 0; i <= (localIndex.x ? 0 : 1); i++)
	for(int j = 0; j <= (localIndex.y ? 0 : 1); j++)
	for(int k = 0; k <= (localIndex.z ? 0 : 1); k++) 
	{
		if(i == 0 && j == 0 && k == 0) continue;
		Chunk* neighbor = it->second->getNeighbor(-glm::ivec3(i,j,k));
		if(!neighbor) continue;
		neighbor->mesh->markVoxelForUpdate(localIndex + chunkSize * glm::ivec3(i,j,k));
	}
	
	unsigned char fluidCellType = it->second->fluidSim.getCell(localIndex).type;

	bool isSolidCell = v.amount > 0;
	if(getTileAt(glm::vec3(index) + 0.1f).cubeMaterialId != 0) isSolidCell = true;
	if(fluidCellType == fluidSimAirType || fluidCellType == fluidSimSolidType)
		it->second->fluidSim.setCell(localIndex, 0, isSolidCell ? fluidSimSolidType : fluidSimAirType);
	
	if(Core::host.isLive()) 
	{
		auto playersInRange = playerManager->getInRange(glm::vec3(index), terrainUpdateRange);
		for(const auto&[id, player] : playersInRange) 
		{
			if(id == PlayerManager::hostPlayerId) continue;
			PeerInfo peer;
			peer.id = id;
			SetVoxelPacket packet;
			packet.amount = v.amount;
			packet.id = v.voxelTypeId;
			packet.x = index.x;
			packet.y = index.y;
			packet.z = index.z;
			Core::host.send(this->getNetworkId(), peer, &packet, sizeof(packet));
		}
	}
}

void World::setVoxelAt(glm::vec3 position, Voxel voxel) 
{
	Chunk* chunk = getChunkAt(position);
	if(!chunk) return;
	glm::ivec3 globalVoxelIndex = vec3ToIndex(position);
	glm::ivec3 voxelIndex = ivec3RealMod(globalVoxelIndex, chunkSize);
	chunk->voxelData.quickSetVoxel(voxelIndex, voxel);
	chunk->dirtyMesh = true;
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	for(int k = -1; k <= 1; k++) 
	{
		Chunk* neighbor = chunk->getNeighbor(glm::ivec3(i,j,k));
		if(neighbor == nullptr) continue;
		if(
			voxelIndex.x == 0 && i == -1 ||
			voxelIndex.y == 0 && j == -1 ||
			voxelIndex.z == 0 && k == -1 ||
			voxelIndex.x == (chunkSize.x - 1) && i == 1 ||
			voxelIndex.y == (chunkSize.y - 1) && j == 1 ||
			voxelIndex.z == (chunkSize.z - 1) && k == 1
		) 
		{
			neighbor->dirtyMesh = true;
		}
	}
	chunk->mesh->markVoxelForUpdate(voxelIndex);
	
	for(int i = 0; i <= (voxelIndex.x ? 0 : 1); i++)
	for(int j = 0; j <= (voxelIndex.y ? 0 : 1); j++)
	for(int k = 0; k <= (voxelIndex.z ? 0 : 1); k++) 
	{
		if(i == 0 && j == 0 && k == 0) continue;
		Chunk* neighbor = chunk->getNeighbor(-glm::ivec3(i,j,k));
		if(!neighbor) continue;
		neighbor->mesh->markVoxelForUpdate(voxelIndex + chunkSize * glm::ivec3(i,j,k));
	}
	
	unsigned char fluidCellType = chunk->fluidSim.getCell(voxelIndex).type;
	bool isSolidCell = voxel.amount > 0;

	if(getTileAt(position).cubeMaterialId != 0) isSolidCell = true;
	if(fluidCellType == fluidSimAirType || fluidCellType == fluidSimSolidType)
		chunk->fluidSim.setCell(voxelIndex, 0, isSolidCell ? fluidSimSolidType : fluidSimAirType);

	if(Core::host.isLive()) 
	{
		auto playersInRange = playerManager->getInRange(position, terrainUpdateRange);
		for(const auto&[id, player] : playersInRange) 
		{
			if(id == PlayerManager::hostPlayerId) continue;
			PeerInfo peer;
			peer.id = id;
			SetVoxelPacket packet;
			packet.amount = voxel.amount;
			packet.id = voxel.voxelTypeId;
			packet.x = globalVoxelIndex.x;
			packet.y = globalVoxelIndex.y;
			packet.z = globalVoxelIndex.z;
			Core::host.send(this->getNetworkId(), peer, &packet, sizeof(packet));
		}
	}
}

void World::setTileAt(glm::vec3 position, const TileMap::TileData& tile) 
{
	position += tileWorldOffset;
	Chunk* chunk = getChunkAt(position);
	if(chunk) 
	{
		glm::ivec3 globalVoxelIndex = vec3ToIndex(position);
		glm::ivec3 voxelIndex = ivec3RealMod(globalVoxelIndex, chunkSize);
		chunk->tileMap.setTileAtIndex(voxelIndex, tile);
		chunk->dirtyTileMesh = true;
		unsigned char fluidType = chunk->fluidSim.getCell(voxelIndex).type;
		if(fluidType != fluidSimSolidType && tile.cubeMaterialId) 
		{
			chunk->fluidSim.setCell(voxelIndex, 0, fluidSimSolidType);
		}
		else if(fluidType == fluidSimSolidType && tile.cubeMaterialId == 0) 
		{
			if(getVoxelAt(position).amount <= 0)
				chunk->fluidSim.setCell(voxelIndex, 0, fluidSimAirType);
		}
		if(Core::host.isLive()) 
		{
			auto playersInRange = playerManager->getInRange(position, terrainUpdateRange);
			for(const auto&[id, player] : playersInRange) 
			{
				if(id == PlayerManager::hostPlayerId) continue;
				PeerInfo peer;
				peer.id = id;
				SetCubeTilePacket packet;
				packet.cubeId = tile.cubeMaterialId;
				packet.x = globalVoxelIndex.x;
				packet.y = globalVoxelIndex.y;
				packet.z = globalVoxelIndex.z;
				Core::host.send(this->getNetworkId(), peer, &packet, sizeof(packet));
			}
		}
	}
}


float World::getValueAt(glm::vec3 position) const
{
	glm::ivec3 chunkIndex = vec3ToIndex(position, chunkSize);
	const Chunk* chunk = getChunkConst(chunkIndex);
	if(!chunk) return -1.0f;
	return chunk->voxelData.getInterpolatedFieldValue(position - glm::vec3(chunkIndex * chunkSize));
}

glm::vec3 World::getNormalAt(glm::vec3 position) const
{
	const Chunk* chunk = getChunkAtConst(position);
	glm::ivec3 chunkIndex = vec3ToIndex(position, chunkSize);
	if(!chunk) return glm::vec3(0.0f, 1.0f, 0.0f);
	return chunk->voxelData.getNormal(position - glm::vec3(chunkIndex * chunkSize));
}

const Chunk* World::getChunkConst(glm::ivec3 index) const
{
	auto it = chunks.find(index);
	return it == chunks.cend() ? nullptr : it->second;
}

Chunk* World::getChunk(glm::ivec3 index)
{
	return const_cast<Chunk*>(getChunkConst(index));
}

void World::loadArea(glm::ivec3 from, glm::ivec3 to, int ticks) 
{
	if(Core::client.isConnected() && !joinComplete) 
	{
		return;
	}
	for(int i = from.x; i <= to.x; i++) 
	{
		for(int j = from.y; j <= to.y; j++) 
		{
			for(int k = from.z; k <= to.z; k++) 
			{
				glm::ivec3 index = glm::ivec3(i,j,k);
				//Create chunk
				//Create chunk load/generate job if this is not a client
				bool isNewChunk = markChunkForLoad(index, ticks, true, !Core::client.isConnected());
				//If this is a client the load job was not created
				//And if chunk was just created we send a chunk data request packet
				//Chunk loader will keep chunks alive while in render distance, as soon as client receives chunk file it will create a normal load job 
				if(Core::client.isConnected() && isNewChunk) 
				{
					ChunkRequestPacket crp = ChunkRequestPacket(index.x, index.y, index.z, VoxelData::getChunkFileLastModify(
						getChunkPath(index)
					));
					Core::client.send({Core::networkDictionary.getNetIdOf("World"), 0}, &crp, sizeof(crp));
				}
			}		
		}
	}
}

void World::disconnectFromTheHost() 
{

}

void World::clientUpdateService() 
{
	// Using constexpr for values that don't change to keep the cave... organized.
	static constexpr unsigned int authRetryInterval = 150;
	static constexpr unsigned int infoRetryInterval = 150;
	static constexpr unsigned int saveRetryInterval = 20;
	static constexpr unsigned int maxRetries = 8;
	static constexpr unsigned int maxSaveRetries = 30;

	if (!playerManager->isAuthentificated()) 
	{
		// Every few beats, we reach out.
		if ((authRetryCounter++ % authRetryInterval) == 0) 
		{
			playerManager->sendAuthPacket();
			Logger::logInfo("Sending auth packet.");
		}

		// If the cycle repeats too many times, the path ends.
		if ((authRetryCounter / authRetryInterval) > maxRetries) 
		{
			Logger::logInfo("Failed to log in to the host.");
			disconnectFromTheHost();
			return;
		}
	}
	else if (!gotWorldInfo) 
	{
		// Looking for the shape of the world.
		if ((infoRetryCounter++ % infoRetryInterval) == 0) 
		{
			WorldInfoRequestPacket packet;
			Core::client.send(this->getNetworkId(), &packet, sizeof(packet));
			Logger::logInfo("Sending wolrd info request.");
		}
		
		if ((infoRetryCounter / infoRetryInterval) > maxRetries) 
		{
			Logger::logInfo("Failed to get world info.");
			disconnectFromTheHost();
			return;
		}
	}
	else if (!storageHasBeenSetup) 
	{
		setupWorldStorage();
	}
	else if(!playerManager->hasOwnPlayer()) 
	{
		if((playerSaveRetryCounter++ % saveRetryInterval) == 0) 
		{
			playerManager->sendPlayerSaveRequestPacket();
			Logger::logInfo("Sending player save data request.");
		}
		if((playerSaveRetryCounter / saveRetryInterval) > maxSaveRetries) 
		{
			Logger::logInfo("Failed to get player save data.");
			disconnectFromTheHost();
			return;
		}
	}
	else if(!joinComplete) 
	{
		Logger::logInfo(std::format("Joined host world with id {}", worldName));
		joinComplete = true;
	}
}

void World::update() 
{
	if(Core::client.isConnected()) 
	{
		clientUpdateService();
		if(!joinComplete) return;
	}
	else 
	if(!storageHasBeenSetup) 
	{
		setupWorldStorage();
	}
	else if (!playerManager->hasOwnPlayer()) 
	{
		playerManager->createPlayer(PlayerManager::hostPlayerId, playerManager->getUUID(), PlayerBuilder::PlayerType::ControlledPlayer);
	}

	/*for(auto[index, chunk] : chunks) 
	{
		if(chunk == nullptr) continue;
		chunk->fluidSim.update(50);
		if(chunk->fluidSim.isTextureDirty()) 
		{
			chunk->fluidSim.uploadTexture();
		}
	}*/
	//wait for water update
	waterWorkFinish.acquire();
	for(auto[index, chunk] : chunks) 
	{
		chunk->fluidSim.stageUpdates();
	} 

	std::vector<glm::ivec3> removeLoadedMarked;
	static int randTick = 0;
	for(auto&[index, ticks] : loadMarkedChunks) 
	{
		ticks -= randTick % 3;
		int temp = randTick;
		randTick++;
		randTick *= 5;
		randTick %= 17;
		if(ticks <= 0) 
		{
			removeLoadedMarked.push_back(index);
		}
	}

///////////////////////////Mesh update enqueing
	int initialQueueSize = meshUpdateQueueQueue.size();
	for(int muqi = 0; muqi < std::min(initialQueueSize, 25); muqi++) 
	{
		Chunk* meshUpdateChunk = meshUpdateQueueQueue.front();
		bool chunksGenerated = true;
		if(meshUpdateChunk->isFreed) 
		{
			meshUpdateQueueQueue.pop();
			chunksGenerated = false;
			goto skip_mesh_update_enqueue;
		} 
		if(meshUpdateChunk->terrainGenerated == false) 
		{
			meshUpdateQueueQueue.pop();
			meshUpdateQueueQueue.emplace(meshUpdateChunk);
			goto skip_mesh_update_enqueue;
		}
		for(int i = -1; i <= 1; i++)
		for(int j = -1; j <= 1; j++)
		for(int k = -1; k <= 1; k++) 
		{
			if(!(i|j|k)) continue;
			if(!chunksGenerated) break;
			Chunk* neighbor = meshUpdateChunk->getNeighbor(glm::ivec3(i,j,k));
			if(!neighbor) 
			{
				chunksGenerated = false;
				break;
			}
			if(!neighbor->terrainGenerated) 
			{
				chunksGenerated = false;
				break;
			}
		}
		if(chunksGenerated) 
		{
			unsigned int threadId = meshGenThreadNextId++ % threadsUsed;
			chunkMeshGenQueuesMutex[threadId].lock();
			chunkMeshGenQueues[threadId].emplace(meshUpdateChunk);
			chunkMeshGenQueuesMutex[threadId].unlock();
			tileMeshUpdateQueue.createJob(meshUpdateChunk);
			meshUpdateQueueQueue.pop();
		}
		else 
		{
			meshUpdateQueueQueue.pop();
			meshUpdateQueueQueue.emplace(meshUpdateChunk);
		}
	}
	skip_mesh_update_enqueue:

////////////////////////////

	while(uploadMeshChunk || meshUploadQueue.get(uploadMeshChunk)) 
	{
		if(uploadMeshChunk->isFreed == false) 
		{
			bool lock = uploadMeshChunk->meshGenMutex.try_lock();
			if(lock) 
			{
				uploadMeshChunk->mesh->uploadMesh();
				//uploadMeshchunk->tileMap.uploadMesh();
				uploadMeshChunk->meshGenMutex.unlock();		
				uploadMeshChunk = nullptr;
			}
		}
		else 
		{
			uploadMeshChunk = nullptr;
		}
	}

	while(uploadTileMeshChunk || tileMeshUploadQueue.get(uploadTileMeshChunk)) 
	{
		if(uploadTileMeshChunk->isFreed == false) 
		{
			bool lock = uploadTileMeshChunk->tileMeshGenMutex.try_lock();
			if(lock) 
			{
				//uploadTileMeshChunk->mesh->uploadMesh();
				uploadTileMeshChunk->tileMap.uploadMesh();
				uploadTileMeshChunk->tileMeshGenMutex.unlock();		
				uploadTileMeshChunk = nullptr;
			}
		}
		else 
		{
			uploadTileMeshChunk = nullptr;
		}
	}
	for(glm::ivec3 index : removeLoadedMarked) 
	{
		if(chunkDestroy(index)) 
		{
			loadMarkedChunks.erase(index);
		}
	}
	//Request water update
	waterWorkRequest.release();

	for(auto&[index, chunk] : chunks) 
	{
		glm::ivec3 renderOffset = glm::abs(index - renderPosition);
		if(
			chunk->dirtyMesh && (++chunk->framesSinceMeshUpdate > 6) && 
			glm::max(glm::max(renderOffset.x, renderOffset.y), renderOffset.z) <= renderDistance
		) 
		{
			chunk->framesSinceMeshUpdate = 0;
			chunk->dirtyMesh = false;
			unsigned int threadId = meshGenThreadNextId++ % threadsUsed;
			chunkMeshGenQueuesMutex[threadId].lock();
			chunkMeshGenQueues[threadId].emplace(chunk);
			chunkMeshGenQueuesMutex[threadId].unlock();
		}
		if(chunk->dirtyTileMesh) 
		{
			chunk->dirtyTileMesh = false;
			tileMeshUpdateQueue.createJob(chunk);
		}
	}

 	// Cast the raw queue into its specific type to see the requests clearly
	chunkReqQueueT& chunkRequests = *((chunkReqQueueT*)(chunkRequestsQueue));

	// Determine how many requests to process now. Not all at once... 
	// just a small handful to keep the world moving smoothly.
	unsigned int requestServingPerFrame = std::min(10, (int)chunkRequests.size());
	unsigned int requestsServed = 0;

	// A loop that continues until the queue is empty or our frame limit is reached
	while(!chunkRequests.empty() && requestsServed++ < requestServingPerFrame) 
	{
		// Pull the first request from the front of the queue
		auto&[peer, request] = chunkRequests.front();
		
		// Transform the coordinates into a 3D integer index
		const glm::ivec3 index = glm::ivec3(request.x, request.y, request.z);

		// Find where the player is standing in the vastness
		const glm::vec3 playerPosition = playerManager->getPlayer(peer.id).node->getRelativeTransform().getPosition();
		
		// Calculate which chunk the player currently inhabits
		const glm::ivec3 originChunk = vec3ToIndex(playerPosition, chunkSize);
		
		// Find the distance between the player and the requested chunk
		const glm::ivec3 offset = glm::abs(index - originChunk);

		// If the chunk is too far away to be seen, let it drift away from the queue
		if(glm::max(offset.x, offset.z) > horizontalRenderDistance || offset.y > verticalRenderDistance) 
		{
			Logger::logInfo(std::format("Chunk {}.{}.{} is out of range for peer id {} at chunk {}.{}.{}", request.x, request.y, request.z, peer.id, originChunk.x, originChunk.y, originChunk.z));
			chunkRequests.pop();
			continue;
		}

		// Search for the chunk in our collection of known things
		auto chunkIt = chunks.find(index);
		if(chunkIt == chunks.cend()) 
		{
			Logger::logInfo(std::format("Chunk {}.{}.{} is not loaded yet.", request.x, request.y, request.z));
			// The chunk is missing. Mark it to be born, and move it to the back of the line
			markChunkForLoad(index, 1000, false, true);
			chunkRequests.emplace(peer, request);
			chunkRequests.pop();
			continue;
		}
		else if(chunkIt->second->terrainGenerated)
		{
			// The chunk is already here... 
			// If it hasn't changed since we last looked, just send a whisper from the cache
			if(request.lastModify == chunkIt->second->voxelData.getLastModify()) 
			{
				LoadFromCachePacket packet(request.x, request.y, request.z);
				Core::host.send(this->getNetworkId(), peer, &packet, sizeof(packet));
				Logger::logInfo(std::format("Sent {}.{}.{} chunk cache packet to peer id {}.", request.x, request.y, request.z, peer.id));
			}
			else 
			{
				// If it has changed, send its full story—the packet and the saved data
				ChunkDataPacket packet(request.x, request.y, request.z, 0, 0, 0);
				unsigned int saveDataSize = 0;
				char* saveData = compressor.compressChunk(chunkIt->second, saveDataSize);
				std::vector<char> buffer(sizeof(packet) + saveDataSize);
				
				// Stitch the packet and data together into a single buffer
				std::memcpy(buffer.data(), &packet, sizeof(packet));
				std::memcpy(buffer.data() + sizeof(packet), saveData, saveDataSize);
				
				Core::host.send(this->getNetworkId(), peer, buffer.data(), buffer.size());
				Logger::logInfo(std::format("Sent {}.{}.{} chunk data to peer id {}.", request.x, request.y, request.z, peer.id));
			}
			// The request has been fulfilled; remove it from the queue
			chunkRequests.pop();
		}
	}
}

void World::setRenderPosition(glm::ivec3 origin) 
{
	this->renderPosition = origin;
}

void World::chunkCreate(glm::ivec3 index, Chunk* chunk) 
{
	chunk->voxelData.setIndex(index);
	//chunk->mesh = new MarchingCubesVoxelMesh(&chunk->voxelData, 0.0f, 1, glm::vec3(0.0f));
	//chunk->node = new GameNode3D();
	chunk->node->getRelativeTransform().setPosition(index * chunkSize);
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	for(int k = -1; k <= 1; k++) 
	{
		if(!(i|j|k)) continue;
		glm::ivec3 side = glm::ivec3(i,j,k);
		Chunk* neighbour = getChunk(index + side);
		if(!neighbour) continue;
		chunk->voxelData.setNeighbor(side, &neighbour->voxelData);
		chunk->setNeighbor(side, neighbour);
		neighbour->voxelData.setNeighbor(-side, &chunk->voxelData);
		neighbour->setNeighbor(-side, chunk);
		if(neighbour->voxelData.getNeighborCount() == 26) 
		{
			meshUpdateQueueQueue.emplace(neighbour);
		}
	}
	this->getParent()->addChild(chunk->node);
}

bool World::chunkDestroy(glm::ivec3 index) 
{
	auto it = chunks.find(index);
	if(it == chunks.cend()) return true;
	Chunk* chunk = it->second;
	chunks.erase(index);
	chunk->meshGenMutex.lock();
	chunk->tileMeshGenMutex.lock();
	//delete chunk->mesh;
	//chunk->tileMap.getParent()->dettachComponent(chunk->tileMap);
	this->getParent()->removeChild(chunk->node);
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	for(int k = -1; k <= 1; k++) 
	{
		if(!(i|j|k)) continue;
		Chunk* n = chunk->getNeighbor(glm::ivec3(i,j,k));
		if(!n) continue;
		if(chunksAllocator.freeLock(n)) 
		{
			n->setNeighbor(-glm::ivec3(i,j,k), nullptr);
			n->voxelData.setNeighbor(-glm::ivec3(i,j,k), nullptr);
			chunksAllocator.freeUnlock(n);
		}
	}
	chunkSaveAndFreeQueueMutex.lock();
	chunkSaveAndFreeQueue.emplace(chunk);
	chunkSaveAndFreeQueueMutex.unlock();
	chunk->meshGenMutex.unlock();
	chunk->tileMeshGenMutex.unlock();
	//glDeleteTextures(1, &chunk->tileMap.volumeSampler);
	return true;
}

bool World::chunkLoad(Chunk* chunk, ChunkCompressor& compressor) 
{
	glm::ivec3 index = chunk->voxelData.getIndex();
	std::filesystem::path chunkPath = getChunkPath(index);
	bool loaded = false;
	chunk->tileMap.clear();
	chunk->fluidSim.clear();
	if(std::filesystem::exists(chunkPath)) 
	{	
		loaded = true;
		std::vector<char> data = loadBytesFromFile(chunkPath);
		compressor.decompressChunk(chunk, data.data(), data.size());
	}
	if(!loaded) 
	{
		if(!Core::client.isConnected()) 
		{
			generateChunk(chunk, index, chunkSize, World::worldData.seed);
		}
		else 
		{
			throw std::runtime_error("Trying to load chunk on a client without receiving data for that chunk.");
		}
	}
	if(!loaded) 
	{
		for(int i = 0; i < chunkSize.x; i++) 
		{
			for(int j = 0; j < chunkSize.y; j++) 
			{
				for(int k = 0; k < chunkSize.z; k++) 
				{
					if(
						chunk->tileMap.getTileAtIndex(glm::ivec3(i,j,k)).cubeMaterialId != 0 || 
						chunk->voxelData.getVoxelConst(glm::ivec3(i,j,k)).amount > 0
					) 
					{
						chunk->fluidSim.setCell(glm::ivec3(i,j,k), 0, fluidSimSolidType, false);
					}
					else
					if(j + index.y * chunkSize.y > 0) 
					{
						chunk->fluidSim.setCell(glm::ivec3(i,j,k), 0, fluidSimAirType, false);
					}
					else 
					{
						//chunk->fluidSim.setCell(glm::ivec3(i,j,k), 255, 1, false);
					}
				}
			}
		}
	}
	/*if(loaded) 
	{
		if(std::filesystem::exists(fluidPath)) 
		{
			auto bytes = loadBytesFromFile(fluidPath);
			std::vector<unsigned short> data;
			data.resize(bytes.size() / sizeof(unsigned short));
			memcpy(data.data(), bytes.data(), bytes.size());
			chunk->fluidSim.decodeCompressedData(data.data(), data.size());
		}
	}*/
	chunk->fluidSim.setTextureDirty();
	return true;
}

void World::chunkLoadThreadMethod(unsigned int id) 
{
	ChunkCompressor compressor;
	while(alive.load(std::memory_order::relaxed)) 
	{
		bool noWork = true;
		Chunk* chunk = nullptr;
		if(chunkLoadQueuesMutex[id].try_lock()) 
		{
			if(chunkLoadQueues[id].size()) 
			{
				chunk = chunkLoadQueues[id].front();
				chunkLoadQueues[id].pop();
			}
			chunkLoadQueuesMutex[id].unlock();
		}
		if(chunk) 
		{
			if(chunksAllocator.freeLock(chunk)) 
			{
				noWork = false;
				chunkLoad(chunk, compressor);
				chunk->terrainGenerated = true;
				chunksAllocator.freeUnlock(chunk);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(noWork ? 500 : 10));
	}
}

void World::chunkSaveThreadMethod() 
{
	ChunkCompressor compressor;
	while(alive.load(std::memory_order::relaxed) || chunkSaveAndFreeQueue.size()) 
	{
		Chunk* saveChunk = nullptr;

		if(chunkSaveAndFreeQueue.size() && chunkSaveAndFreeQueueMutex.try_lock()) 
		{
			saveChunk = chunkSaveAndFreeQueue.front();
			chunkSaveAndFreeQueue.pop();
			chunkSaveAndFreeQueueMutex.unlock();
		}
		if(saveChunk) 
		{
			if(saveChunk->terrainGenerated) 
			{
				//saveChunk->voxelData.save(getChunkPath(saveChunk->voxelData.getIndex()));
				unsigned int voxelSaveSize = 0;
				char* chunkData = compressor.compressChunk(saveChunk, voxelSaveSize);
				std::ofstream chunkFile(getChunkPath(saveChunk->voxelData.getIndex()), std::ios::binary);
				chunkFile.write(chunkData, voxelSaveSize);
				//if(!saveChunk->tileMap.isEmpty()) saveChunk->tileMap.save(getTilesPath(saveChunk->voxelData.getIndex()));
				/*if(saveChunk->fluidSim.getTotalFluid()) 
				{
					int fluidDataSize = 0;
					auto fluidPath = getFluidPath(saveChunk->voxelData.getIndex());
					const unsigned short* fluidData = saveChunk->fluidSim.compressedData(fluidDataSize);
					std::ofstream file(fluidPath, std::ios::binary);
					file.write((char*)fluidData, fluidDataSize * sizeof(unsigned short));
				}*/
			}
			//delete saveChunk->tileMap;
			//delete saveChunk->fluidSim;
			//delete saveChunk;
			chunksAllocator.freeChunk(saveChunk);
		}

		
		std::this_thread::sleep_for(std::chrono::milliseconds(chunkSaveAndFreeQueue.size() ? 10 : 500));
	}
}

void World::meshGenThreadMethod(unsigned int id) 
{
	while(alive.load(std::memory_order::relaxed)) 
	{
		bool noWork = true;
		Chunk* chunk = nullptr;
		chunkMeshGenQueuesMutex[id].lock();
		if(chunkMeshGenQueues[id].size()) 
		{
			chunk = *chunkMeshGenQueues[id].begin();
			chunkMeshGenQueues[id].erase(chunk);
		}
		chunkMeshGenQueuesMutex[id].unlock();
		if(chunk != nullptr) 
		{
			noWork = false;
			if(chunksAllocator.freeLock(chunk)) 
			{
				std::lock_guard<std::mutex> meshGenLock(chunk->meshGenMutex);	
				for(int i = -1; i <= 1; i++)
				for(int j = -1; j <= 1; j++)
				for(int k = -1; k <= 1; k++)
				{
					if((i|j|k) == 0) continue;
					Chunk* n = chunk->getNeighbor(glm::ivec3(i,j,k));
					if(n && n->isFreed) 
					{
						chunk->setNeighbor(glm::ivec3(i,j,k), nullptr);
						chunk->voxelData.setNeighbor(glm::ivec3(i,j,k), nullptr);
					}
				}
				chunk->mesh->updateMesh();
				meshUploadQueue.add(chunk);
				chunk->dirtyMesh = false;
				chunksAllocator.freeUnlock(chunk);
			}
		}
		if(tileMeshUpdateQueue.tryGetJob(chunk)) 
		{
			noWork = false;
			if(chunksAllocator.freeLock(chunk)) 
			{
				std::lock_guard<std::mutex> meshGenLock(chunk->tileMeshGenMutex);
				for(int i = -1; i <= 1; i++)
				for(int j = -1; j <= 1; j++)
				for(int k = -1; k <= 1; k++)
				{
					if((i|j|k) == 0) continue;
					Chunk* n = chunk->getNeighbor(glm::ivec3(i,j,k));
					if(n && n->isFreed) 
					{
						chunk->setNeighbor(glm::ivec3(i,j,k), nullptr);
						chunk->voxelData.setNeighbor(glm::ivec3(i,j,k), nullptr);
					}
				}
				chunk->tileMap.updateMesh();
				tileMeshUploadQueue.add(chunk);
				chunk->dirtyTileMesh = false;
				chunksAllocator.freeUnlock(chunk);
			}
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(noWork ? 50 : 5));
	}
}

bool World::rayCast(glm::vec3& hit, glm::vec3 dir, int maxSteps, float maxDistance, glm::ivec3& hitIndex, glm::vec3& normal) 
{
	glm::vec3 voxelHit = hit;
	glm::vec3 cubeHit = hit + tileWorldOffset;
	glm::ivec3 voxelHitIndex;
	glm::ivec3 cubeHitIndex;
	glm::vec3 voxelNormal;
	glm::vec3 cubeNormal;
	float voxelHitLength;
	float cubeHitLength;
	bool vhit = voxelCast(voxelHit, dir, maxSteps, maxDistance, voxelHitIndex, voxelNormal, voxelHitLength);
	bool chit = cubeCast(cubeHit, dir, maxSteps, maxDistance, cubeHitIndex, cubeNormal, cubeHitLength);
	//false - voxel, true - cube
	bool hitChoice = false;
	if(vhit && chit) 
	{
		hitChoice = voxelHitLength > cubeHitLength;
	}
	else 
	{
		hitChoice = chit;
	}
	if(hitChoice) 
	{
		hit = cubeHit - tileWorldOffset;
		hitIndex = cubeHitIndex;
		normal = cubeNormal;
	}
	else 
	{
		hit = voxelHit;
		hitIndex = voxelHitIndex;
		normal = voxelNormal;
	}
	return vhit || chit;
}

bool World::voxelCast(glm::vec3& hit, glm::vec3 dir, int maxSteps, float maxDistance, glm::ivec3& hitIndex, glm::vec3& normal, float& rayLength) 
{
	if(dir.x == 0.0f && dir.y == 0.0f && dir.z == 0.0f) 
	{
		return false;
	}
	dir = glm::normalize(dir);
	hitIndex = vec3ToIndex(hit);
	glm::vec3 absdir = glm::abs(dir);
	rayLength = 0.0f;
	int stepCount = 0;
	while(stepCount < maxSteps && rayLength < maxDistance) 
	{
		glm::vec3 fract = hit - glm::vec3(hitIndex);
		float xLength = dir.x ? (((dir.x > 0.0f) ? (1.0f - fract.x) : (fract.x)) / absdir.x) : (FLT_MAX);
		float yLength = dir.y ? (((dir.y > 0.0f) ? (1.0f - fract.y) : (fract.y)) / absdir.y) : (FLT_MAX);
		float zLength = dir.z ? (((dir.z > 0.0f) ? (1.0f - fract.z) : (fract.z)) / absdir.z) : (FLT_MAX);
		if(xLength < yLength && xLength < zLength) 
		{
			hit += xLength * dir;
			hitIndex.x += dir.x > 0.0f ? 1 : -1;
			rayLength += xLength;
		}
		else if(yLength < zLength) 
		{
			hit += yLength * dir;
			hitIndex.y += dir.y > 0.0f ? 1 : -1;
			rayLength += yLength;
		}
		else 
		{
			hit += zLength * dir;
			hitIndex.z += dir.z > 0.0f ? 1 : -1;
			rayLength += zLength;
		}
		float value = getValueAt(hit);
		const float delta = 0.005f;
		if(value > 0.0f) 
		{
			float p2 = getValueAt(hit - dir * delta);
			if(p2 < value) 
			{
				float depth = (value - p2) / delta;
				depth = depth > 1.0f ? 1.0f : depth;
				hit -= dir * depth;
				rayLength -= depth;
				for(int substeps = 0; substeps < 25 && getValueAt(hit) < 0.0f; substeps++) 
				{
					hit += dir * depth * 0.08f;
					rayLength += depth * 0.08f;
				}
			}
			normal = getNormalAt(hit);
			glm::ivec3 hitIndexBase = hitIndex;
			float closest = 16.0f;
			for(int cx = 0; cx <= 1; cx++) 
			for(int cy = 0; cy <= 1; cy++) 
			for(int cz = 0; cz <= 1; cz++) 
			{
				glm::vec3 p = glm::vec3(cx, cy, cz) - glm::fract(hit);
				float l2 = p.x * p.x + p.y * p.y + p.z * p.z;
				glm::ivec3 vindex = hitIndexBase + glm::ivec3(cx, cy, cz);
				if(l2 < closest && getVoxel(vindex).amount > 0) 
				{
					closest = l2;
					hitIndex = vindex;
				}
			}
			return true;
		}
		stepCount++;
	}
	return false;
}
bool World::cubeCast(glm::vec3& hit, glm::vec3 dir, int maxSteps, float maxDistance, glm::ivec3& hitIndex, glm::vec3& normal, float& rayLength) 
{
	if(dir.x == 0.0f && dir.y == 0.0f && dir.z == 0.0f) 
	{
		return false;
	}
	dir = glm::normalize(dir);
	hitIndex = vec3ToIndex(hit);
	glm::vec3 absdir = glm::abs(dir);
	rayLength = 0.0f;
	int stepCount = 0;
	while(stepCount < maxSteps && rayLength < maxDistance) 
	{
		glm::vec3 fract = hit - glm::vec3(hitIndex);
		float xLength = dir.x ? (((dir.x > 0.0f) ? (1.0f - fract.x) : (fract.x)) / absdir.x) : (FLT_MAX);
		float yLength = dir.y ? (((dir.y > 0.0f) ? (1.0f - fract.y) : (fract.y)) / absdir.y) : (FLT_MAX);
		float zLength = dir.z ? (((dir.z > 0.0f) ? (1.0f - fract.z) : (fract.z)) / absdir.z) : (FLT_MAX);
		if(xLength < yLength && xLength < zLength) 
		{
			hit += xLength * dir;
			hitIndex.x += dir.x > 0.0f ? 1 : -1;
			rayLength += xLength;
			normal.x = dir.x > 0.0f ? -1.0f : 1.0f;
			normal.y = 0.0f;
			normal.z = 0.0f;
		}
		else if(yLength < zLength) 
		{
			hit += yLength * dir;
			hitIndex.y += dir.y > 0.0f ? 1 : -1;
			rayLength += yLength;
			normal.x = 0.0f;
			normal.y = dir.y > 0.0f ? -1.0f : 1.0f;
			normal.z = 0.0f;
		}
		else 
		{
			hit += zLength * dir;
			hitIndex.z += dir.z > 0.0f ? 1 : -1;
			rayLength += zLength;
			normal.x = 0.0f;
			normal.y = 0.0f;
			normal.z = dir.z > 0.0f ? -1.0f : 1.0f;
		}
		glm::ivec3 chunkIndex = vec3ToIndex(hit, chunkSize);
		Chunk* chunk = getChunk(chunkIndex);
		unsigned short tileCube = chunk ? chunk->tileMap.cubeAtWrapped(hitIndex - chunkIndex * chunkSize) : 0;
		const float delta = 0.001f;
		if(tileCube) 
		{
			return true;
		}
		stepCount++;
	}
	return false;
}

int World::getLoadingChunksCount() 
{
	unsigned int jobCount = 0;
	for(int i = 0; i < threadsUsed; i++) 
	{
		chunkLoadQueuesMutex[i].lock();
		jobCount += chunkLoadQueues[i].size();
		chunkLoadQueuesMutex[i].unlock();
	}
	return jobCount;
}


void World::onClientPacket(const void* data, int size) 
{
	if(size < sizeof(WorldPacketHeader)) return;
	switch ((WorldPacketType)((WorldPacketHeader*)(data))->packetType)
	{
	case WorldPacketType::ChunkData:
		//chunk data will be received
		//we will save it to file
		//then we try to load the chunk normally as if we were in singleplayer
		if(size < sizeof(ChunkDataPacket)) return;

		{
			ChunkDataPacket& cdp = *(ChunkDataPacket*)data;
			if(logChunkPackets) Logger::logInfo(std::format("Got chunk {}.{}.{} data packet.", cdp.x, cdp.y, cdp.z));
			char* chunkDataBytes = ((char*)data) + sizeof(ChunkDataPacket);
			std::filesystem::path path = getChunkPath(glm::ivec3(cdp.x, cdp.y, cdp.z));
			std::ofstream file(path, std::ios::binary);
			file.write(chunkDataBytes, size - sizeof(ChunkDataPacket));
			createLoadJobForChunkAtIndex(glm::ivec3(cdp.x, cdp.y, cdp.z));
		}
		break;
	case WorldPacketType::ChunkUnload:
		//unload chunk
		//Don't need it yet 
		//Might not need it at all
		break;
	case WorldPacketType::LoadFromCache:
		//
		if(size != sizeof(LoadFromCachePacket)) return;
		//
		{
			LoadFromCachePacket& lfcp = *(LoadFromCachePacket*)data;
			createLoadJobForChunkAtIndex(glm::ivec3(lfcp.x, lfcp.y, lfcp.z));
		}
		break;
	default:
	case WorldPacketType::WorldInfo:
		if(size != sizeof(WorldInfoPacket)) return;
		
		{
			WorldInfoPacket& wip = *(WorldInfoPacket*)data;
			for(int i = 0; i < worldIdSize; i++) 
			{
				const char c = wip.id[i];
				if((c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '.' && c != '-' && c != '_') 
				{
					Logger::logInfo("Server sent invalid world id format.");
					return;
				}
			}
			worldName = std::string((const char*)wip.id, worldIdSize);
			memcpy(worldData.id, wip.id, worldIdSize);
			gotWorldInfo = true;
			Logger::logInfo(std::format("Got valid world info packet, world id: {}", worldName));
		}
	break;
	case WorldPacketType::SetCubeTile:
		if(!joinComplete) return;
		if(size != sizeof(SetCubeTilePacket)) return;
		{
			SetCubeTilePacket packet = *(const SetCubeTilePacket*)data;
			TileMap::TileData tile;
			tile.cubeMaterialId = packet.cubeId;
			tile.orientation = 0;
			tile.tileId = 0;
			setTileAt(glm::vec3(packet.x, packet.y, packet.z), tile);
		}
		break;
	case WorldPacketType::SetVoxel:
		if(!joinComplete) return;
		if(size != sizeof(SetVoxelPacket)) return;
		{
			SetVoxelPacket packet = *(const SetVoxelPacket*)data;
			Voxel voxel;
			voxel.voxelTypeId = packet.id;
			voxel.amount = packet.amount;
			setVoxel(glm::ivec3(packet.x, packet.y, packet.z), voxel);
		}
		break;
		return;
		break;
	}
}
void World::onHostPeerPacket(PeerInfo& peer, const void* data, int size) 
{
	if(size < sizeof(WorldPacketHeader)) return;
	switch ((WorldPacketType)((WorldPacketHeader*)(data))->packetType)
	{
	case WorldPacketType::RequestChunk:
		if(size != sizeof(ChunkRequestPacket)) return;
		{
			ChunkRequestPacket& packet = *((ChunkRequestPacket*)data);
			if(logChunkPackets) Logger::logInfo(std::format("Chunk {}.{}.{} requested.", packet.x, packet.y, packet.z));
			((chunkReqQueueT*)(chunkRequestsQueue))->emplace(std::pair<PeerInfo, ChunkRequestPacket>(peer, *((ChunkRequestPacket*)data)));
		}
		break;
	case WorldPacketType::WorldInfoRequest:
		{
			WorldInfoPacket packet;
			memcpy(packet.id, worldData.id, worldIdSize);
			Core::host.send({Core::networkDictionary.getNetIdOf("World"), 0}, peer, &packet, sizeof(packet));
		}
		break;
	default:
		return;
		break;
	}
}

bool World::markChunkForLoad(glm::ivec3 index, int ticks, bool rendered, bool loadJob) 
{
	bool wasEmplaced = false;
	if(loadMarkedChunks.find(index) == loadMarkedChunks.cend()) 
	{
		//this emplacement only fails when chunk already exists
		//if it ever fails for any other reason there are bigger problems that shouldn't be looked for here
		auto[it, emplaced] = chunks.try_emplace(index, nullptr);
		wasEmplaced = emplaced;
		if(emplaced) 
		{
			it->second = chunksAllocator.newChunk();//new Chunk(chunkSize, index);
			chunkCreate(index, it->second);
		}
		if(emplaced) 
		{
			if(loadJob) 
			{
				unsigned int threadId = (this->chunkLoadThreadNextId++) % threadsUsed;
 				chunkLoadQueuesMutex[threadId].lock();
				chunkLoadQueues[threadId].push(it->second);
				chunkLoadQueuesMutex[threadId].unlock();
			}
		}
	}
	int& loadTicks = loadMarkedChunks[index];
	loadTicks = glm::max(ticks, loadTicks);
	return wasEmplaced;
}

void World::createLoadJobForChunkAtIndex(glm::ivec3 index) 
{
	auto it = chunks.find(index);
	if(it != chunks.cend()) 
	{
		unsigned int threadId = (this->chunkLoadThreadNextId++) % threadsUsed;
 		chunkLoadQueuesMutex[threadId].lock();
		chunkLoadQueues[threadId].push(it->second);
		chunkLoadQueuesMutex[threadId].unlock();
	}
}

void World::onHostPeerConnect(PeerInfo& peer) 
{
}

//maps every integer to a unique string
//does nothing else
std::string chunkIndexEncode(int index) 
{
	if(index == 0) return "0"; 
	std::string str = "";
	bool n = index < 0;
	if(n) index = -index;
	int len = 1;
	unsigned int tempind = index;
	const char basedict[] = "0123456789abcdefghijklmnopqrstuvwxyz_";
	const int base = sizeof(basedict) - 1;
	while(tempind) 
	{
		tempind /= base;
		len++;
	}
	str.reserve(len);
	
	while(index) 
	{
		str += basedict[index % base];
		index /= base;
	}
	if(n) str += "-";
	return str;
}

std::filesystem::path World::getChunkPath(glm::ivec3 index) const
{
	return getWorldPath() / 
	chunksDirectory /
	std::format("{}.{}.{}", chunkIndexEncode(index.x), chunkIndexEncode(index.y), chunkIndexEncode(index.z));
}
std::filesystem::path World::getTilesPath(glm::vec3 index) const
{
	return getWorldPath() / 
	chunksDirectory /
	std::format("{},{},{}", chunkIndexEncode(index.x), chunkIndexEncode(index.y), chunkIndexEncode(index.z));
}
std::filesystem::path World::getFluidPath(glm::vec3 index) const
{
	return getWorldPath() / 
	chunksDirectory /
	std::format("{}~{}~{}", chunkIndexEncode(index.x), chunkIndexEncode(index.y), chunkIndexEncode(index.z));
}
std::filesystem::path World::getWorldPath() const
{
	return worldsDirectory /
	(Core::client.isConnected() ? "remote" : "local") /
	worldName;
}
void World::saveAll() 
{
	saving.store(true, std::memory_order_release);
	for(auto[ind, chunk] : chunks) 
	{
		chunkDestroy(chunk->voxelData.getIndex());
	}
	auto playerSaveData = playerBuilder->saveBuffer();
	if(!std::filesystem::exists(getPlayersDirectory())) std::filesystem::create_directories(getPlayersDirectory());
	std::ofstream playerSaveFile(getPlayersDirectory() / playerManager->getUUID());
	playerSaveFile.write(playerSaveData.data(), playerSaveData.size());
}

std::filesystem::path World::getPlayersDirectory() const 
{
	return getWorldPath() /  "players";
}

void World::waterUpdateThreadMethod() 
{
	while(alive.load(std::memory_order::relaxed)) 
	{
		waterWorkRequest.acquire();
		for(auto[index, chunk] : chunks) 
		{
			if(chunk == nullptr) continue;
			chunk->fluidSim.update(100);
		}
		waterWorkFinish.release();
	}
}

bool cullCheck(const FrustumPlanes& frustum, glm::vec3 p, float r) 
{
	return (
		glm::dot(p, frustum.bottomNormal) - frustum.bottomD < r &&
		glm::dot(p, frustum.upNormal) - frustum.upD < r &&
		glm::dot(p, frustum.farNormal) - frustum.farD < r &&
		glm::dot(p, frustum.nearNormal) - frustum.nearD < r &&
		glm::dot(p, frustum.leftNormal) - frustum.leftD < r &&
		glm::dot(p, frustum.rightNormal) - frustum.rightD < r
	);
}

void World::render(Kirino::StateColletion& state, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) 
{
	glm::vec3 camPos = Core::camera->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition();
	this->material->useProgram();
	this->material->setupTextures(state.textureBindings);
	this->material->setupUniformBlocks(state.uboBindings);
	for(const auto&[index, chunk] : chunks) 
	{
		bool renderableMarchingCubesMesh = chunk->mesh->getMesh().vao != nullptr;
		if(renderableMarchingCubesMesh && *chunk->mesh->getMesh().vao == 0) renderableMarchingCubesMesh = false;
		if(renderableMarchingCubesMesh && chunk->mesh->getMesh().indexCount == 0) renderableMarchingCubesMesh = false;

		bool hasTileMesh = false;
		for(unsigned int s = 0; s < 6; s++) 
		{
			if(chunk->tileMap.meshParts[s]->quadCount != 0) 
			{
				hasTileMesh = true;
				break;
			}
		}

		bool doCullCheck = renderableMarchingCubesMesh ||
		chunk->fluidSim.getTotalFluid() ||
		hasTileMesh;
		
		if(!doCullCheck) continue;
		glm::vec3 position = glm::vec3(index * chunkSize);
		chunk->frustumCulling = cullCheck(frustum, position + 16.0f, 30.0f);

		if(renderableMarchingCubesMesh == false) continue;
		if(!chunk->frustumCulling) continue;
		material->setUniform("offset", &position, sizeof(position), 0);
		material->setupUniforms();
		glBindVertexArray(*chunk->mesh->getMesh().vao);
		glDrawElements(GL_TRIANGLES, chunk->mesh->getMesh().indexCount, GL_UNSIGNED_INT, 0);
	}
		
	this->grassMaterial->useProgram();
	this->grassMaterial->setupTextures(state.textureBindings);
	this->grassMaterial->setupUniformBlocks(state.uboBindings);
	glDisable(GL_CULL_FACE);
	for(const auto&[index, chunk] : chunks) 
	{
		if(!chunk->frustumCulling) continue;
		if(!chunk->mesh->hasGrass()) continue;
		glm::vec3 position = glm::vec3(index * chunkSize);
		float planeF = 0.0f;
		float planeR = 1.0f;
		grassMaterial->setUniform(grassMaterialOffsetIndex, &position, sizeof(position), 0);
		grassMaterial->setUniform(grassMaterialPlaneIndex, &planeF, sizeof(planeF), 0);
		grassMaterial->setupUniforms();
		chunk->mesh->drawGrass();
		grassMaterial->setUniform(grassMaterialPlaneIndex, &planeR, sizeof(planeF), 0);
		grassMaterial->setupUniforms();
		chunk->mesh->drawGrass();
	}
	glEnable(GL_CULL_FACE);

	this->tilemapMaterial->useProgram();
	this->tilemapMaterial->setupUniformBlocks(state.uboBindings);
	for(const auto&[index, chunk] : chunks) 
	{
		if(!chunk->frustumCulling) continue;
		if(chunk->tileMap.isEmpty()) continue;
		this->tilemapMaterial->setTexture("materialVolume", &chunk->tileMap.volumeSampler, 1, 0, true);
		this->tilemapMaterial->setupTextures(state.textureBindings);
		glm::vec3 position = glm::vec3(index * chunkSize) - tileWorldOffset;
		tilemapMaterial->setUniform("offset", &position, sizeof(position), 0);
		for(unsigned int s = 0; s < 6; s++) 
		{
			if(chunk->tileMap.meshParts[s]->quadCount == 0) continue;
			Kirino::VertexArrayObject& vao = chunk->tileMap.meshParts[s]->getVAO();
			if(vao == 0) continue;
			float normalOffset = position[2u - (s >> 1u)];
			float cameraOffset = camPos[2u - (s >> 1u)];
			if((s % 2) ?
			cameraOffset < normalOffset :
			(cameraOffset - 32.0f) > normalOffset
			) continue; 

			tilemapMaterial->setUniform("s", &s, sizeof(s), 0);
			tilemapMaterial->setupUniforms();
			glBindVertexArray(vao);
			glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, chunk->tileMap.meshParts[s]->quadCount);
		}
	}
}	
void World::shadowRender(Kirino::StateColletion& state, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) 
{
	this->shadowMaterial->useProgram();
	this->shadowMaterial->setupTextures(state.textureBindings);
	this->shadowMaterial->setupUniformBlocks(state.uboBindings);
	for(const auto&[index, chunk] : chunks) 
	{
		if(chunk->mesh->getMesh().vao == nullptr) continue;
		if(*chunk->mesh->getMesh().vao == 0) continue;
		if(chunk->mesh->getMesh().indexCount == 0) continue;
		glm::vec3 position = glm::vec3(index * chunkSize);
		if(!cullCheck(frustum, position + 16.0f, 28.0f)) continue;
		shadowMaterial->setUniform("offset", &position, sizeof(position), 0);
		shadowMaterial->setupUniforms();
		glBindVertexArray(*chunk->mesh->getMesh().vao);
		glDrawElements(GL_TRIANGLES, chunk->mesh->getMesh().indexCount, GL_UNSIGNED_INT, 0);
	}

	this->tilemapShadowMaterial->useProgram();
	this->tilemapShadowMaterial->setupTextures(state.textureBindings);
	this->tilemapShadowMaterial->setupUniformBlocks(state.uboBindings);
	for(const auto&[index, chunk] : chunks) 
	{
		glm::vec3 position = glm::vec3(index * chunkSize) - tileWorldOffset;
		tilemapShadowMaterial->setUniform("offset", &position, sizeof(position), 0);
		for(unsigned int s = 0; s < 6; s++) 
		{
			if(chunk->tileMap.meshParts[s]->quadCount == 0) continue;
			Kirino::VertexArrayObject& vao = chunk->tileMap.meshParts[s]->getVAO();
			if(vao == 0) continue;
			if(!chunk->frustumCulling) continue;
			tilemapShadowMaterial->setUniform("s", &s, sizeof(s), 0);
			tilemapShadowMaterial->setupUniforms();
			glBindVertexArray(vao);
			glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, chunk->tileMap.meshParts[s]->quadCount);
		}
	}
}

void World::transparentRender(Kirino::StateColletion& state, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) 
{
	glm::vec3 camPos = Core::camera->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition();
	glm::vec3 camDir = Core::camera->getParentOfTypeConst<GameNode3D>()->getWorldTransform().forward();

	waterFBO.bind();
	waterFBO.apply();
	glDepthMask(true);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	//glBindVertexArray(waterCubeVAO);
	int w = Core::camera->getWidth() / 6.0f;
	int h = Core::camera->getHeight() / 6.0f;
	if(waterRenderTexture.getWidth() != w || waterRenderTexture.getHeight() != h) 
	{
		waterFBO.viewport(w, h);
		waterRenderTexture.create(w, h);
		waterDepthTexture.create(w, h);
		waterNormalTexture.create(w, h);
	} 

	waterMaterial->useProgram();
	//waterMaterial->setSingleTextureByIndex(waterMaterialRenderFragPosIndex, renderTextures.position);
	waterMaterial->setupUniformBlocks(state.uboBindings);
	for(const auto&[index, chunk] : chunks) 
	{
		glm::vec3 offset = glm::vec3(index * chunkSize) - tileWorldOffset;
		float screenSpace = 0.0f;
		if(
			camPos.x > offset.x - 0.01f && 
			camPos.y > offset.y - 0.01f && 
			camPos.z > offset.z - 0.01f && 
			camPos.x < (offset.x + 32.01f) && 
			camPos.y < (offset.y + 32.01f) && 
			camPos.z < (offset.z + 32.01f)
		)
		{
			screenSpace = 1.0f;
		}
		chunk->fluidSim.uploadTexture();
		waterMaterial->setUniform(waterMaterialOffsetIndex, &offset, sizeof(offset), 0);
		waterMaterial->setUniform(waterMaterialScreenSpaceIndex, &screenSpace, sizeof(screenSpace), 0);
		waterMaterial->setSingleTextureByIndex(waterMaterialFluidSamplerIndex, chunk->fluidSim.fluidSampler, true);
		waterMaterial->setupTextures(state.textureBindings);
		waterMaterial->setupUniforms();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}	
	waterFBO.unbind();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthMask(false);
	glDisable(GL_BLEND);
	unsigned int waterTextureId = waterRenderTexture.id();
	unsigned int waterNormalTextureId = waterNormalTexture.id();
	waterCombineMaterial->setTexture("WaterSampler", &waterTextureId);
	waterCombineMaterial->setTexture("FragColorMap", &renderTextures.color);
	waterCombineMaterial->setTexture("PositionSampler", &renderTextures.position);
	waterCombineMaterial->setTexture("WaterNormalSampler", &waterNormalTextureId);
	
	static float waterAnimationTime = 0.0f;
	waterAnimationTime += 0.05f;
	if(waterAnimationTime >= 64.0f) waterAnimationTime = 0.0f; 
	float waterAnimationFrame = floor(waterAnimationTime);
	waterCombineMaterial->setUniform("frame", &waterAnimationFrame, sizeof(waterAnimationFrame), 0);
	waterCombineMaterial->useProgram();
	waterCombineMaterial->setupUniforms(); 
	waterCombineMaterial->setupUniformBlocks(state.uboBindings);
	waterCombineMaterial->setupTextures(state.textureBindings);
	glBindVertexArray(*BasicMeshes::screenSquare.vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//material->setupUniformBlocks(ub);	
	/*
	This is displayed later
	unsigned int waterRenderTextureId = waterRenderTexture.id();
	waterCombineMaterial->setTexture("textureImage", &waterRenderTextureId, 1, 0);
	*/
}

void World::setupWorldStorage() 
{
	//create folder if nonexistent
	if(!std::filesystem::exists(getWorldPath())) 
	{
		std::filesystem::create_directories(getWorldPath());
	}
	if(!std::filesystem::exists(getWorldPath() / chunksDirectory)) 
	{
		std::filesystem::create_directories(getWorldPath() / chunksDirectory);
	}
	//create world save data if nonexistent
	if(!std::filesystem::exists(getWorldPath() / saveDataFile)) 
	{
		std::ofstream worldData(getWorldPath() / saveDataFile, std::ios::binary);
		worldData.write((char*)&World::worldData, sizeof(WorldSaveData));
	}
	//read save data if exists
	else 
	{
		std::ifstream worldData(getWorldPath() / saveDataFile, std::ios::binary);
		worldData.read((char*)&World::worldData, sizeof(WorldSaveData));
	}
	if(!std::filesystem::exists(getPlayersDirectory())) 
	{
		std::filesystem::create_directories(getPlayersDirectory());
	}
	storageHasBeenSetup = true;
}
EXPORT_COMPONENT(World);