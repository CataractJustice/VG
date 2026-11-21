#include "Inventory/InventoryManager.h"
#include "Inventory/InventoryPackets/InventorySlotDataPacket.h"
#include "Inventory/InventoryPackets/InventorySlotTryMovePacket.h"
#include "Inventory/InventoryPackets/Types.h"
#include "Core.h"
#include "Inventory/InventorySlot.h"
void InventoryManager::handleSlotMoveHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(size != sizeof(InventorySlotTryMovePacket)) return;
	const InventorySlotTryMovePacket& request = *((const InventorySlotTryMovePacket*)data);	
	auto slotFromIt = slots.find(request.slotFrom);
	auto slotToIt = slots.find(request.slotTo);
	if(slotFromIt == slots.cend()) return;
	if(slotToIt == slots.cend()) return;
	InventorySlot* slotFrom = slotFromIt->second;
	InventorySlot* slotTo = slotToIt->second;
	InventoryItem itemFrom = slotFrom->getItem();
	InventoryItem itemTo = slotTo->getItem();
	if(itemFrom.isEmpty()) return;
	if(
		!slotFrom->playerAccessCheck(peer.id, InventorySlotAccessType::take) || 
		!slotTo->playerAccessCheck(peer.id, InventorySlotAccessType::place)
	) return;
	if(!slotTo->acceptsItem(itemFrom.getId())) return;
	if(
		!itemTo.isEmpty() &&
		itemFrom.getTemplatePtr() != itemTo.getTemplatePtr()
	) 
	{
		if(
			!slotTo->playerAccessCheck(peer.id, InventorySlotAccessType::take) ||
			!slotFrom->playerAccessCheck(peer.id, InventorySlotAccessType::place)
		) return;
		if(!slotFrom->acceptsItem(itemTo.getId())) return;
		slotFrom->getItem() = itemTo;
		slotTo->getItem() = itemFrom;
	}
	else 
	{
		unsigned int pushed = slotTo->pushStack(itemFrom);
		if(pushed == 0) return;
		slotFrom->getItem().amount -= pushed;
	}
	slotFrom->sync();
	slotTo->sync();
}