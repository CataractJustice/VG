#include "Player/PlayerManager.h"
#include "System/Logger.h"

void PlayerManager::handleEquippSlotHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(size != sizeof(PlayerEquippSlotPacket)) return;
	auto playerIt = players.find(peer.id);
	if(playerIt == players.cend()) return;
	auto& [id, player] = *playerIt;
	PlayerEquippSlotPacket& packet = *((PlayerEquippSlotPacket*)data);
	player.builder->itemEquipper->selectItemSlot(packet.slot);
}