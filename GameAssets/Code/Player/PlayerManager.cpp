#include "PlayerManager.h"
#include "GameNode/GameNode3D.h"
#include "Physics/Bodies/RigidBody.h"
#include "GameNode/ComponentFactory.h"
#include "System/Random.h"
#include <fstream>
#include "System/Logger.h"
#include "PlayerManagerPackets.h"
#include "PlayerSpawner.h"
#include "World.h"
const char allowedUUIDChars[] = "0123456789qwertyuiopasdfghjklzxcvbnm-_.";

PlayerManager::PlayerManager() 
{
	usernameLoaded = false;
	uuidLoaded = false;
	authentificated = false;
	ownPlayerCreated = false;
	memset(username, 0, usernameSize);
	memset(uuid, 0, uuidSize);
	memset(key, 0, keySize);
	authType = PlayerAuthType::None;
}
void PlayerManager::onClientPacket(const void* data, int size) 
{
	const unsigned char packetType = *((const unsigned char*)data);
	if(packetType == PlayerManagerPacketType::authresponse) 
	{
		handleAuthResposeClient(data, size);
	}
	if(packetType == PlayerManagerPacketType::saveData) 
	{
		handleSaveDataClient(data, size);
	}
};

void PlayerManager::onHostPeerPacket(PeerInfo& peer, const void* data, int size) 
{
	unsigned char packetType = *(unsigned char*)data;

	switch (packetType)
	{
	case PlayerManagerPacketType::authentificate:
		handleAuthHost(peer, data, size);
		break;
	case PlayerManagerPacketType::saveRequest:
		handleSaveRequestHost(peer, data, size);
		break;
	case PlayerManagerPacketType::movement:
		handleMovementHost(peer, data, size);
		break;
	case PlayerManagerPacketType::equippSlot:
		handleEquippSlotHost(peer, data, size);
		break;
	case PlayerManagerPacketType::useItem:
		handleUseItemHost(peer, data, size);
		break;
	case PlayerManagerPacketType::craft:
		handleCraftHost(peer, data, size);
		break;
	default:
		Logger::logInfo(std::format("Unknown PlayerManager packet type {}.", packetType));
		break;
	}
};
void PlayerManager::onHostPeerConnect(PeerInfo& peer) 
{
	Logger::logInfo(std::format("New peer connection, id {}.", peer.id));
};
void PlayerManager::onHostPeerDisconnect(PeerInfo& peer) 
{
	Logger::logInfo(std::format("Peer id {} disconnected.", peer.id));
};

const std::string PlayerManager::getUUID() 
{
	if(uuidLoaded) 
	{
		return std::string(uuid, uuidSize);
	}
	const std::filesystem::path profilePath = "./data/profile/";
	const std::filesystem::path uuidPath = profilePath / "uuid";
	if(std::filesystem::exists(uuidPath)) 
	{
		std::ifstream file(uuidPath);
		file.read(uuid, uuidSize);
	}
	else 
	{
		randomBytes(uuid, uuidSize);
		for(int i = 0; i < uuidSize; i++) uuid[i] = allowedUUIDChars[uuid[i] % (sizeof(allowedUUIDChars) - 1)];
		if(!std::filesystem::exists(profilePath)) std::filesystem::create_directories(profilePath);
		std::ofstream ofile(uuidPath);
		ofile.write(uuid, uuidSize);
	}

	uuidLoaded = true;
	return std::string(uuid, uuidSize);
}

const char* PlayerManager::getUsername() 
{
	if(usernameLoaded) 
	{
		return username;
	}
	const std::filesystem::path profilePath = "./data/profile/";
	const std::filesystem::path usernamePath = profilePath / "username";
	if(std::filesystem::exists(usernamePath)) 
	{
		std::ifstream file(usernamePath);
		file.read(username, usernameSize);
	}
	else 
	{
		const unsigned int suffixLength = 13;

		memset(username, 0, sizeof(username));
		
		randomBytes(username, suffixLength);
		
		char allowedChars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
		
		for(int i = 0; i < suffixLength; i++) username[i] = allowedChars[username[i] % (sizeof(allowedChars) - 1)];
		
		char usernameBase[] = "Char-";
		memcpy(username, usernameBase, sizeof(usernameBase) - 1);
		
		if(!std::filesystem::exists(profilePath)) std::filesystem::create_directories(profilePath);
		
		std::ofstream ofile(usernamePath);
		ofile.write(username, usernameSize);
	}

	usernameLoaded = true;
	return username;
}

const char* PlayerManager::getKey() 
{
	memset(key, 0, sizeof(key));
	return key;
}

void PlayerManager::createPlayer(int peerId, const std::string& uuid, PlayerBuilder::PlayerType type) 
{
	if(type == PlayerBuilder::PlayerType::ControlledPlayer) ownPlayerCreated = true;
	PlayerData player;
	GameNode3D* node = new GameNode3D();
	PlayerBuilder* pb = new PlayerBuilder();
	pb->playerType = type;
	node->attachComponent(pb);
	PlayerSpawner* spawner = new PlayerSpawner(node, pb, uuid);
	player.node = node;	
	player.builder = pb;
	players[peerId] = player;
	ownPlayer = &players[peerId];
	GameNode3D* spawnerNode = new GameNode3D();
	pb->build(type);
	pb->itemEquipper->playerId = peerId;
	spawnerNode->getRelativeTransform().setPosition(glm::vec3(0.0f, 256.0f, 0.0f));
	Core::getRoot()->addChild(spawnerNode);
	spawnerNode->attachComponent(spawner);

	

	World::currentWorld->inventoryManager->registerInventory(pb->inventory, std::format("Player[{}]", peerId));
	for(int i = 0; i < hotbarSlotsCount; i++) 
	{
		pb->hotbarSlots[i]->setPlayerAccess(peerId, InventorySlotAccessType::all);
		World::currentWorld->inventoryManager->registerSlot(pb->hotbarSlots[i], std::format("Player[{}].hotbar[{}]", peerId, i));
	}
}

void PlayerManager::sendAuthPacket() 
{
	if(authentificated) return;
	getUUID();
	getUsername();
	PlayerAuthentificatePacket packet;
	packet.packetType = PlayerManagerPacketType::authentificate;
	memcpy(packet.key, key, keySize);
	memcpy(packet.username, username, usernameSize);
	memcpy(packet.uuid, uuid, uuidSize);
	Core::client.send({Core::networkDictionary.getNetIdOf("PlayerManager"), 0}, &packet, sizeof(packet));
}

void PlayerManager::sendPlayerSaveRequestPacket() 
{
	if(ownPlayerCreated) throw std::logic_error("Own player already created.");
	PlayerSaveRequestPacket request;
	request.packetType = PlayerManagerPacketType::saveRequest;
	Core::client.send(this->getNetworkId(), &request, sizeof(request));
}

void PlayerManager::sendPlayerSave(int peerId) 
{
	if(players[peerId].node->getParent() == nullptr) 
	{
		Logger::logInfo(std::format("Player of peer {} is logged in but not yet spawned, not sending player save data.", peerId));
		return;
	}
	PlayerSavePacket packet;
	packet.packetType = PlayerManagerPacketType::saveData;
	packet.connectionId = peerId;
	std::vector<char> saveData = players[peerId].builder->saveBuffer();
	std::vector<char> buffer;
	buffer.resize(sizeof(packet) + saveData.size());

	std::memcpy(buffer.data(), &packet, sizeof(packet));
	std::memcpy(buffer.data() + sizeof(packet),saveData.data(), saveData.size());
	PeerInfo peer;
	peer.id = peerId;
	Core::host.send(this->getNetworkId(), peer, buffer.data(), buffer.size());
}

std::vector<std::pair<int, const PlayerManager::PlayerData*>> PlayerManager::getInRange(glm::vec3 from, float range) const
{
	const float range2 = range * range;
	std::vector<std::pair<int, const PlayerManager::PlayerData*>> inRange;
	for(const auto&[iteratedId, player] : players) 
	{
		if(glm::distance2(from, player.node->getWorldTransform().getPosition()) < range2) 
		{
			inRange.push_back({iteratedId, &player});
		}
	}
	return inRange;
}

std::vector<std::pair<int, const PlayerManager::PlayerData*>> PlayerManager::getInRange(unsigned int id, float range) const
{
	float range2 = range * range;
	auto playerIt = players.find(id);
	if(playerIt == players.cend()) throw std::invalid_argument("PlayerManager::getInRange(id, range), id is not existing player");
	glm::vec3 position = playerIt->second.node->getWorldTransform().getPosition();
	std::vector<std::pair<int, const PlayerManager::PlayerData*>> inRange;
	for(const auto&[iteratedId, player] : players) 
	{
		if(iteratedId == id) continue;
		if(glm::distance2(position, player.node->getWorldTransform().getPosition()) < range2) 
		{
			inRange.push_back({iteratedId, &player});
		}
	}
	return inRange;
}
EXPORT_COMPONENT(PlayerManager);