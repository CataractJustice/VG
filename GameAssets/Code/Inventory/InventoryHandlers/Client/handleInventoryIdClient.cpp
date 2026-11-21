#include "Inventory/InventoryPackets/InventoryInventoryIdPacket.h"
#include "Inventory/InventoryManager.h"

void InventoryManager::handleInventoryIdClient(const void* data, unsigned int size) 
{
	if(size != sizeof(InventoryInventoryIdPacket)) return;
	const InventoryInventoryIdPacket& packet = *((const InventoryInventoryIdPacket*)data);
	if(strnlen(packet.name, sizeof(packet.name)) == sizeof(packet.name)) return;
	std::string name(packet.name);
	auto invIt = namedInventories.find(name);
	if(invIt == namedInventories.cend()) return;
	invIt->second.first = packet.id;
	inventories.insert_or_assign(packet.id, invIt->second.second);
}