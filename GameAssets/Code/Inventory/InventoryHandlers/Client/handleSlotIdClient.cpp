#include "Inventory/InventoryPackets/InventorySlotIdPacket.h"
#include "Inventory/InventoryManager.h"
#include "System/Logger.h"
#include <format>
void InventoryManager::handleSlotIdClient(const void* data, unsigned int size) 
{
	if(size != sizeof(InventorySlotIdPacket)) return;
	const InventorySlotIdPacket& packet = *((const InventorySlotIdPacket*)data);
	if(strnlen(packet.name, sizeof(packet.name)) == sizeof(packet.name)) return;
	std::string name(packet.name);
	auto slotIt = namedSlots.find(name);
	if(slotIt == namedSlots.cend()) return;
	slotIt->second.first = packet.id;
	slots[packet.id] = slotIt->second.second;
	Logger::logInfo(std::format("Got slot named {} id: {}", name, packet.id));
}