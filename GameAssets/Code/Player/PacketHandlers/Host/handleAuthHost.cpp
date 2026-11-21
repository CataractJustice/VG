#include "Player/PlayerManager.h"
#include "System/Logger.h"
unsigned char PlayerManager::authPlayer(PlayerAuthentificatePacket& packet) 
{
	if(authType == PlayerAuthType::None) 
	{
		for(int i = 0; i < uuidSize; i++) 
		{
			const char c = packet.uuid[i];
			if((c < '0' || c > '9') && (c < 'a' || c > 'z') && c != '-' && c != '_' && c != '.') 
				return PlayerAuthRejectStatus::uuidFormat;
		}
		bool usernameTerminated = false;
		for(int i = 0; i < usernameSize; i++) 
		{
			const char c = static_cast<char>(
				std::tolower(static_cast<unsigned char>(packet.username[i]))
			);

			if(c)
			{
				if(usernameTerminated) 
					return PlayerAuthRejectStatus::usernameFormat;
				if((c < '0' || c > '9') && (c < 'a' || c > 'z') && c != '-' && c != '_' && c != '.') 
					return PlayerAuthRejectStatus::usernameFormat;
			}
			else 
			{
				usernameTerminated = true;
			}
			packet.username[i] = c;
		}
		if(!usernameTerminated)
			return PlayerAuthRejectStatus::usernameFormat;
		for(const auto&[id, player] : players) 
		{
			if(memcmp(player.uuid, packet.uuid, uuidSize) == 0) 
			{
				return PlayerAuthRejectStatus::uuidExists;
			}
			if(strcmp(player.username, packet.username) == 0) 
			{
				return PlayerAuthRejectStatus::usernameExists;
			}
		}
		return PlayerAuthRejectStatus::ok;
	}
	//
	return PlayerAuthRejectStatus::credentials;
}

void PlayerManager::handleAuthHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(sizeof(PlayerAuthentificatePacket) != size) return;
	PlayerAuthentificatePacket packet = *(PlayerAuthentificatePacket*)(data);
	unsigned char authStatus = PlayerAuthRejectStatus::ok; 
	if(players.find(peer.id) != players.cend()) 
	{
		authStatus = PlayerAuthRejectStatus::peerLoggedIn;
	}
	else 
	{
		authStatus = authPlayer(packet);
	}
	if(authStatus == PlayerAuthRejectStatus::ok) 
	{
		std::string uuidStr = std::string(packet.uuid, uuidSize);
		createPlayer(peer.id, uuidStr, PlayerBuilder::PlayerType::HostSidePlayer);
		PlayerData& player = players[peer.id];
		memcpy(player.username, packet.username, usernameSize);
		memcpy(player.uuid, packet.uuid, uuidSize);
		Logger::logInfo(std::format("Connection id {} logged as {}, with uuid {}", peer.id, player.username, uuidStr));
	}
	else 
	{
		Logger::logInfo(std::format("Connection id {} authentification rejection, reject code: {}.", peer.id, authStatus));
	}
	PlayerAuthenrificateResponsePacket response;
	response.packetType = PlayerManagerPacketType::authresponse;
	response.reject = authStatus;
	Core::host.send({Core::networkDictionary.getNetIdOf("PlayerManager"), 0}, peer, &response, sizeof(response));
	return;
}