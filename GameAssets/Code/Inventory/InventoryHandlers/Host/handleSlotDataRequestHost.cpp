#include "Inventory/InventoryManager.h"
#include "Inventory/InventoryPackets/InventoryRequestSlotDataPacket.h"
#include "Inventory/InventoryPackets/InventorySlotDataPacket.h"
#include "Inventory/InventoryPackets/Types.h"
#include "Core.h"
#include "Inventory/InventorySlot.h"
void InventoryManager::handleSlotDataRequestHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(size != sizeof(InventoryRequestSlotDataPacket)) return;
	const InventoryRequestSlotDataPacket& request = *((const InventoryRequestSlotDataPacket*)data);	
	auto slotIt = slots.find(request.id);
	if(slotIt == slots.cend()) return;
	InventorySlot* slot = slotIt->second;
	if(!slot->playerAccessCheck(peer.id, InventorySlotAccessType::view)) return;
	InventorySlotDataPacket response;
	response.packetType = InventoryManagerPacketType::slotData;
	response.id = request.id;
	std::vector<char> saveBuffer = slot->saveBuffer();
	std::vector<char> buffer(sizeof(response) + saveBuffer.size());
	memcpy(buffer.data(), &response, sizeof(response));
	memcpy(buffer.data() + sizeof(response), saveBuffer.data(), saveBuffer.size());
	Core::host.send(this->getNetworkId(), peer, buffer.data(), buffer.size());
}