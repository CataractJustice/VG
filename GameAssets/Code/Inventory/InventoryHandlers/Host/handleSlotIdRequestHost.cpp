#include "Inventory/InventoryManager.h"
#include "Inventory/InventoryPackets/InventorySlotIdPacket.h"
#include "Inventory/InventoryPackets/InventoryRequestSlotIdPacket.h"
#include "Inventory/InventoryPackets/Types.h"
#include "Core.h"
#include "System/Logger.h"
void InventoryManager::handleSlotIdRequestHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(size != sizeof(InventoryRequestSlotIdPacket)) return;
	const InventoryRequestSlotIdPacket& request = *((const InventoryRequestSlotIdPacket*)data);	
	if(strnlen(request.name, sizeof(request.name)) == sizeof(request.name)) return;
	std::string name(request.name);
	Logger::logInfo(std::format("Searching for slot {}.", name));
	auto slotIt = namedSlots.find(name);
	if(slotIt == namedSlots.cend()) return;
	unsigned int id = slotIt->second.first;
	Logger::logInfo(std::format("Slot {} found under id {}.", name, id));
	InventorySlotIdPacket response;
	response.packetType = InventoryManagerPacketType::slotId;
	memcpy(response.name, request.name, sizeof(response.name));
	response.id = id;
	Core::host.send(this->getNetworkId(), peer, &response, sizeof(response));
}