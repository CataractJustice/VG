#include "Inventory/InventoryPackets/InventorySlotDataPacket.h"
#include "Inventory/InventoryManager.h"
#include "Inventory/InventorySlot.h"
#include "System/Logger.h"
#include <format>
void InventoryManager::handleSlotDataClient(const void* data, unsigned int size) 
{
	if(size <= sizeof(InventorySlotDataPacket)) return;
	const InventorySlotDataPacket& packet = *((const InventorySlotDataPacket*)data);
	const char* slotData = ((const char*)data) + sizeof(InventorySlotDataPacket);
	auto slotIt = slots.find(packet.id);
	if(slotIt == slots.cend()) return;
	slotIt->second->loadBufferSafe(slotData, size - sizeof(InventorySlotDataPacket));
	Logger::logInfo(std::format("Got slot data, id: {}", packet.id));
}