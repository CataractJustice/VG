#include "Player/PlayerManager.h"
#include "Crafting/PlayerCrafting.h"
void PlayerManager::handleCraftHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(size != sizeof(PlayerCraftByIndexPacket)) return;
	auto playerIt = players.find(peer.id);
	if(playerIt == players.cend()) return;
	auto& [id, player] = *playerIt;
	PlayerCraftByIndexPacket& packet = *((PlayerCraftByIndexPacket*)data);
	player.builder->crafting->tryCraftByRecipeIndex(packet.craftIndex);
}