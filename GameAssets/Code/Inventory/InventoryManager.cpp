#include "InventoryManager.h"
#include "Core.h"
#include "InventoryPackets/Types.h"
#include "System/Logger.h"
#include "InventoryPackets.h"
#include "InventorySlot.h"
NetworkId InventoryManager::invManagerNetId;
InventoryManager::InventoryManager() 
{
	invManagerNetId = this->getNetworkId();
	inventoryIncrementalID = 38;
	slotIncrementalID = 38;
}
void InventoryManager::onClientPacket(const void* data, int size) 
{
	const unsigned char packetType = *((const unsigned char*)data);
	switch(packetType) 
	{
	case InventoryManagerPacketType::inventoryData:
		handleInventoryDataClient(data, size);
		break;
	case InventoryManagerPacketType::inventoryId:
		handleInventoryIdClient(data, size);
		break;
	case InventoryManagerPacketType::slotData:
		handleSlotDataClient(data, size);
		break;
	case InventoryManagerPacketType::slotId:
		handleSlotIdClient(data, size);
		break;
	default:
		Logger::logInfo(std::format("Unknown InventoryManager packet type {}", packetType));
	}
} 
void InventoryManager::onHostPeerPacket(PeerInfo& peer, const void* data, int size) 
{
	const unsigned char packetType = *((const unsigned char*)data);
	switch(packetType) 
	{
	case InventoryManagerPacketType::requestSlotId:
		handleSlotIdRequestHost(peer, data, size);
		break;
	case InventoryManagerPacketType::requestInventoryId:
		handleInventoryIdRequestHost(peer, data, size);
		break;
	case InventoryManagerPacketType::requestSlotData:
		handleSlotDataRequestHost(peer, data, size);
		break;
	case InventoryManagerPacketType::slotTryMove:
		handleSlotMoveHost(peer, data, size);
		break;
	default:
		Logger::logInfo(std::format("Unknown InventoryManager packet type {}", packetType));
	}
}

unsigned int InventoryManager::registerSlot(InventorySlot* slot, const std::string& name) 
{
	if(Core::host.isLive())
	{
		slotIncrementalID++;
		const unsigned int id = slotIncrementalID;
		slot->slotRegisteredId = id;
		this->namedSlots[name] = {id, slot};
		this->slots[id] = slot;
		return id;
	}
	else if(Core::client.isConnected())
	{
		this->namedSlots[name] = {0, slot};
		InventoryRequestSlotIdPacket request;
		request.packetType = InventoryManagerPacketType::requestSlotId;
		memset(request.name, 0, sizeof(request.name));
		memcpy(request.name, name.data(), name.length());
		Core::client.send(this->getNetworkId(), &request, sizeof(request));
	}
	return 0;
}
unsigned int InventoryManager::registerInventory(Inventory* inventory, const std::string& name) 
{
	if(Core::host.isLive())
	{
		inventoryIncrementalID++;
		const unsigned int id = inventoryIncrementalID;
		this->namedInventories[name] = {id, inventory};
		this->inventories[id] = inventory;
		return id;
	}
	else if(Core::client.isConnected())
	{
		this->namedInventories[name] = {0, inventory};
		InventoryRequestInventoryIdPacket request;
		request.packetType = InventoryManagerPacketType::requestInventoryId;
		memset(request.name, 0, sizeof(request.name));
		memcpy(request.name, name.data(), name.length());
		Core::client.send(this->getNetworkId(), &request, sizeof(request));
	}
	return 0;
}