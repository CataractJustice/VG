#pragma once
#include "GameNode/Component.h"
#include "libs/glm/gtc/quaternion.hpp"
#include <map>
#include "GameNode/GameNode3D.h"
#include "PlayerBuilder.h"
#include <filesystem>
#include "PlayerManagerPackets.h"
namespace TealEngine 
{
	class RigidBody;
}
using namespace TealEngine;

class PlayerManager : public Component 
{
	private:
	PlayerAuthType authType;
	struct PlayerData 
	{
		char username[usernameSize];
		char uuid[uuidSize];
		GameNode3D* node;
		PlayerBuilder* builder;
	};
	std::map<int, PlayerData> players;
	PlayerData* ownPlayer;
	char uuid[uuidSize + 1];
	char key[keySize + 1];
	char username[usernameSize + 1];
	bool uuidLoaded;
	bool usernameLoaded;
	bool authentificated;
	bool ownPlayerCreated;
	std::filesystem::path playerSavePath;
	unsigned char authPlayer(PlayerAuthentificatePacket& packet);
	public:
	const std::string getUUID();
	const char* getUsername();
	const char* getKey();
	PlayerManager();
	void handleAuthHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleSaveRequestHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleMovementHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleEquippSlotHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleGetSlotInfoHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleUseItemHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleCraftHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleAuthResposeClient(const void* data, unsigned int size);
	void handleSaveDataClient(const void* data, unsigned int size);
	void handleSlotInfoClient(const void* data, unsigned int size);
	void onClientPacket(const void* data, int size) override;
	void onHostPeerPacket(PeerInfo& peer, const void* data, int size) override;
	void onHostPeerConnect(PeerInfo& peer) override;
	void onHostPeerDisconnect(PeerInfo& peer) override;
	void createPlayer(int peerId, const std::string& uuid, PlayerBuilder::PlayerType type);
	void sendAuthPacket();
	void sendPlayerSaveRequestPacket();
	void sendPlayerSave(int peerId);
	std::vector<std::pair<int, const PlayerData*>> getInRange(glm::vec3 from, float range) const;
	std::vector<std::pair<int, const PlayerData*>> getInRange(unsigned int id, float range) const;
	inline PlayerData& getPlayer(unsigned int peerId) 
	{
		if(players.find(peerId) == players.cend()) throw std::runtime_error(std::format("Peer id {} has no player data", peerId));
		return players[peerId];
	}
	inline bool isAuthentificated() const { return authentificated; }
	inline bool hasOwnPlayer() const { return ownPlayerCreated; }
	static const unsigned int hostPlayerId = 20032102;
};