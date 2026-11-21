#pragma once
#include "InventoryItem.h"
#include <unordered_set>
#include "Network/Connection.h"
class Inventory;
class InventorySlotComponent;
namespace InventorySlotAccessType 
{
	const unsigned char view = 1 << 0;
	const unsigned char place = 1 << 1;
	const unsigned char take = 1 << 2;
	const unsigned char all = view | place | take;
}

class InventorySlot 
{
	private:
	InventoryItem item;
	bool picturePresent;
	std::string currentPicture;
	Inventory* inventory;
	std::unordered_map<unsigned int, unsigned char> playersAccess;
	InventorySlotComponent* slotComponent;
	public:
	unsigned int slotRegisteredId;
	bool useWhiteList = false;
	std::unordered_set<ItemTemplateId> whiteList = {};
	std::unordered_set<ItemTemplateId> blackList = {};
	inline bool acceptsItem(const ItemTemplateId& id) const
	{
		return useWhiteList ? whiteList.contains(id) : !blackList.contains(id);
	}
	InventorySlot();
	InventoryItem& getItem();
	const InventoryItem& getItemConst() const;
	//returns the amount of item that was added to the current stack
	int pushStack(InventoryItem stack);
	int pushStackCheck(InventoryItem stack);
	std::vector<char> saveBuffer();
	void loadBuffer(const char* bufferPtr);
	bool loadBufferSafe(const char* buffer, unsigned int size);
	const Inventory* getInventoryConst() const;
	Inventory* getInventory();
	inline void setInventory(Inventory* inv) { inventory = inv; }
	void sync();

	void addPlayerAccess(unsigned int peerId, unsigned char mask);
	void removePlayerAccess(unsigned int peerId, unsigned char mask);
	void setPlayerAccess(unsigned int peerId, unsigned char mask);
	
	bool playerAccessCheck(unsigned int peerId, unsigned char mask);
	inline void setSlotComponent(InventorySlotComponent* ptr) { this->slotComponent = ptr; }
	~InventorySlot();
};