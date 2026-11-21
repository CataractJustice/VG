#include "Player/PlayerManager.h"

void PlayerManager::handleGetSlotInfoHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(size != sizeof(PlayerGetSlotInfoPacket)) return;
	auto playerIt = players.find(peer.id);
	if(playerIt == players.cend()) return;
	const PlayerGetSlotInfoPacket& packet = *((PlayerGetSlotInfoPacket*)data);
	auto& [id, player] = *playerIt;
	InventorySlot* slot = nullptr;
	switch (packet.slotType)
	{
	case PlayerSlotInfoPacketSlotType::hotbar:
		if(packet.slotIndex >= hotbarSlotsCount) return;
		slot = player.builder->hotbarSlots[packet.slotIndex];
		break;
	case PlayerSlotInfoPacketSlotType::backpack:
		return;
		break;
	case PlayerSlotInfoPacketSlotType::container:
		return;
		break;
	default:
		return;
		break;
	}
	if(!slot) return;
	if(packet.subSlotIndex != notSubSlot) 
	{
		if(packet.subSlotIndex >= slot->getItem().slots.size()) return;
		slot = slot->getItem().slots[packet.subSlotIndex].get();
	}
	std::vector<char> slotData = slot->saveBuffer();
	std::vector<char> buffer(sizeof(PlayerGetSlotInfoPacket) + slotData.size());
	memcpy(buffer.data(), &packet, sizeof(packet));
	memcpy(buffer.data() + sizeof(packet), slotData.data(), slotData.size());
	Core::host.send(this->getNetworkId(), peer, buffer.data(), buffer.size());
}