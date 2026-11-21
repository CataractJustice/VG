#include "Inventory/InventoryManager.h"
#include "Inventory/InventoryPackets/InventoryInventoryIdPacket.h"
#include "Inventory/InventoryPackets/InventoryRequestInventoryIdPacket.h"
#include "Inventory/InventoryPackets/Types.h"
#include "Core.h"
void InventoryManager::handleInventoryIdRequestHost(PeerInfo& peer, const void* data, unsigned int size) 
{
	if(size != sizeof(InventoryRequestInventoryIdPacket)) return;
	const InventoryRequestInventoryIdPacket& request = *((const InventoryRequestInventoryIdPacket*)data);	
	if(strnlen(request.name, sizeof(request.name)) == sizeof(request.name)) return;
	std::string name(request.name);
	auto invIt = namedInventories.find(name);
	if(invIt == namedInventories.cend()) return;
	unsigned int id = invIt->second.first;
	InventoryInventoryIdPacket response;
	response.packetType = InventoryManagerPacketType::inventoryId;
	memcpy(response.name, request.name, sizeof(response.name));
	response.id = id;
	Core::host.send(this->getNetworkId(), peer, &response, sizeof(response));
}