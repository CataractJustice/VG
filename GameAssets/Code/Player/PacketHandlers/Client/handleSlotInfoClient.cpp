#include "Player/PlayerManager.h"
#include "World.h"

void PlayerManager::handleSlotInfoClient(const void* data, unsigned int size) 
{
	if(size <= sizeof(PlayerSlotInfoPacket)) return;
	const PlayerSlotInfoPacket& header = *((const PlayerSlotInfoPacket*)data);
	ownPlayer->builder->hotbarSlots[header.slotIndex]->loadBufferSafe(((const char*)data) + sizeof(PlayerSlotInfoPacket), size - sizeof(PlayerSlotInfoPacket));
}