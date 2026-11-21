#include "Inventory/InventoryPackets/InventoryDataPacket.h"
#include "Inventory/InventoryManager.h"
#include "Inventory/Inventory.h"
void InventoryManager::handleInventoryDataClient(const void* data, unsigned int size) 
{
	if(size <= sizeof(InventoryDataPacket)) return;
	const InventoryDataPacket& packet = *((const InventoryDataPacket*)data);
	const char* invData = ((const char*)data) + sizeof(InventoryDataPacket);
	auto invIt = inventories.find(packet.invId);
	if(invIt == inventories.cend()) return;
	invIt->second->loadBufferSafe(invData, size - sizeof(InventoryDataPacket));
}