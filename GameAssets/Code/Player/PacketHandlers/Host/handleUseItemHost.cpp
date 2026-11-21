#include "Player/PlayerManager.h"
#include "ItemBehaviors/NetworkItemBase.h"
void PlayerManager::handleUseItemHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(size < sizeof(PlayerUseItemPacket)) return;
	auto playerIt = players.find(peer.id);
	if(playerIt == players.cend()) return;
	PlayerData& player = playerIt->second;
	PlayerUseItemPacket& packet = *((PlayerUseItemPacket*)data);
	if(packet.updateType != ItemUpdateType::packet && size != sizeof(PlayerUseItemPacket)) return; 
	if(packet.usageType >= ItemUsageType::maxUsageType) return;
	if(packet.hotbarIndex >= hotbarSlotsCount) return;
	if(player.builder->itemEquipper->getSlotIndex() != packet.hotbarIndex) return;
	if(player.builder->itemEquipper->getEquipedItemStack().isEmpty()) return;
	if(player.builder->itemEquipper->getItemObject()) 
	{
		auto items = player.builder->itemEquipper->getItemObject()->findComponentsByType<NetworkItemBase>(true);
		for(auto item : items) 
		{
			switch (packet.updateType)
			{
			case ItemUpdateType::packet:
				item->processPacketHost(((const char*)data) + sizeof(PlayerUseItemPacket), size - sizeof(PlayerUseItemPacket));
				break;
			case ItemUpdateType::hold:
				item->holdRefresh(packet.usageType);
				break;
			case ItemUpdateType::press:
				item->doPress(packet.usageType);
				break;
			case ItemUpdateType::release:
				item->doRelease(packet.usageType);
				break;
			default:
				return;
			}
		}
	}
}