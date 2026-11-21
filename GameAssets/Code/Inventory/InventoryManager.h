#pragma once
#include "GameNode/Component.h"
#include <unordered_map>
#include <unordered_set>
class Inventory;
class InventorySlot;
using namespace TealEngine;
class InventoryManager : public Component 
{
	private:
	std::unordered_map<std::string, std::pair<unsigned int, InventorySlot*>> namedSlots;
	std::unordered_map<std::string, std::pair<unsigned int, Inventory*>> namedInventories;
	std::unordered_map<unsigned int, InventorySlot*> slots;
	std::unordered_map<unsigned int, Inventory*> inventories;
	unsigned int slotIncrementalID;
	unsigned int inventoryIncrementalID;
	public:
	static NetworkId invManagerNetId;
	InventoryManager();
	
	void handleSlotDataClient(const void* data, unsigned int size);
	void handleSlotIdClient(const void* data, unsigned int size);
	void handleInventoryDataClient(const void* data, unsigned int size);
	void handleInventoryIdClient(const void* data, unsigned int size);

	void handleSlotMoveHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleSlotIdRequestHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleSlotDataRequestHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleInventoryDataRequestHost(PeerInfo& peer, const void* data, unsigned int size);
	void handleInventoryIdRequestHost(PeerInfo& peer, const void* data, unsigned int size);

	void onClientPacket(const void* data, int size) override; 
	void onHostPeerPacket(PeerInfo& peer, const void* data, int size) override;

	/*
		Planned architecture
		Each inventory and or slot has a player access priviledges information, a list of allowed actions that a player can perform on it
		If player is not in a list then it has no rights
		A player could be able to view the slot or interact with it

		Any action on items like dropping, using, consuming or anything that is not directly related to the storage of item ids and counts:
		-Must not be done in the inventory manager
		-Is done though item behaviour or some other system

		Inventory manager is supposed to be only responsible for inventory contents being sent to clients
		and clients being able to move items between slots if allowed

		Now about existing problems and usages
		Currently we only have player inventory, backpack (empty with no slots), and hotbar
		hotbar item equipping is done through ItemEquipper and is unrelated to the storage synchronization
		Also the initial state of the hotbar and backpakc is also already synchronized through player save data
		Player already knows its own id
		So what we just need to do right now is to call 
		`registerSlot(hotbarSlot[n], std::format("Player.hotbar[{}]", n))` for every hotbar slot when creating player
		and
		`registerInventory(inventory, std::format("Player[{}]", myId))`
		Now what registering slot should do?
		On host it must assign a new id to the slot and save that slot into maps under new id key and under the provided name key
		On client it must only save that slot into the name key map
		Same for inventory
		
		When the client joins it gets all its inventory slot info from save data
		Then it requests the inventory id by name
		Which should make host send the inventory ID and all Inventory IDs of that inventory

	*/

	unsigned int registerSlot(InventorySlot* slot, const std::string& name);
	unsigned int registerInventory(Inventory* inventory, const std::string& name);

};