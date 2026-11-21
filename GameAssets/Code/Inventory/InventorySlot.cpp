#include "InventorySlot.h"
#include "GameNode/ComponentFactory.h"
#include "libs/glm/gtc/type_ptr.hpp"
#include "Core.h"
#include "GameNode/GameNodeGUI.h"
#include "Inventory.h"
#include "Graphics/GUI/Background.h"
#include "Graphics/GUI/MonospaceBitmapText.h"
#include "Core.h"
#include "InventoryManager.h"
#include "InventoryPackets.h"
#include "System/Logger.h"
#include "World.h"
#include "InventorySlot.h"

InventoryItem& InventorySlot::getItem() 
{
	return this->item;
}
const InventoryItem& InventorySlot::getItemConst() const
{
	return this->item;
}
int InventorySlot::pushStack(InventoryItem stack) 
{
	int pushedAmount = pushStackCheck(stack);
	if(this->item.isEmpty() && pushedAmount) 
	{
		this->item = stack;
	}
	else
	if(stack.getTemplatePtr() == this->item.getTemplatePtr()) 
	{
		this->item.amount += pushedAmount;	
	}
	sync();
	return pushedAmount;
}

int InventorySlot::pushStackCheck(InventoryItem stack) 
{
	if(this->useWhiteList) 
	{
		if(this->whiteList.find(stack.getId()) == this->whiteList.cend()) return 0;
	} 
	else
	{
		if(this->blackList.find(stack.getId()) != this->blackList.cend()) return 0;
	}

	if(this->item.isEmpty()) 
	{
		return std::min(stack.getTemplatePtr()->maxStack, stack.amount);
	}

	if(stack.getTemplatePtr() == this->item.getTemplatePtr()) 
	{
		int freeSpace = this->item.getTemplatePtr()->maxStack - this->item.amount;
		int deltaAmount = glm::min(stack.amount, freeSpace);
		return deltaAmount;
	}

	return 0;
}

InventorySlot::InventorySlot() : item("", 0), inventory(nullptr), slotComponent(nullptr)
{
}

std::vector<char> InventorySlot::saveBuffer() 
{
	std::vector<char> buff;
	buff.resize(sizeof(int) + this->item.getId().length() + 1);
	*((int*)buff.data()) = this->item.amount;
	memcpy(buff.data() + sizeof(int), this->item.getId().c_str(), this->item.getId().length() + 1);
	return buff;
}

void InventorySlot::loadBuffer(const char* buffer) 
{
	this->item.init(buffer + sizeof(int), *(int*)buffer);
}

bool InventorySlot::loadBufferSafe(const char* buffer, unsigned int size) 
{
	if(size <= sizeof(int)) return false;
	const int maxLen = size - sizeof(int);
	size_t len = strnlen(buffer + sizeof(int), maxLen);
	if(len >= maxLen) return false;
	if(!InventoryItemTemplates::getInstance().getById(buffer + sizeof(int))) return false;
	loadBuffer(buffer);
	return true;
}

Inventory* InventorySlot::getInventory() 
{
	return inventory;
}

const Inventory* InventorySlot::getInventoryConst() const 
{
	return inventory;
}

void InventorySlot::addPlayerAccess(unsigned int peerId, unsigned char mask) 
{
	if(mask == 0) throw std::invalid_argument("InventorySlot::addPlayerAccess: Adding empty mask");
	auto accessIt = playersAccess.find(peerId);
	if(accessIt == playersAccess.cend()) playersAccess.emplace(peerId, mask);
	else 
	accessIt->second |= mask;
}
void InventorySlot::removePlayerAccess(unsigned int peerId, unsigned char mask) 
{
	if(mask == 0) throw std::invalid_argument("InventorySlot::removePlayerAccess: Removing empty mask");
	auto accessIt = playersAccess.find(peerId);
	if(accessIt == playersAccess.cend()) throw std::invalid_argument("InventorySlot::removePlayerAccess: Removing access from player with no access");
	accessIt->second &= ~mask;
}
void InventorySlot::setPlayerAccess(unsigned int peerId, unsigned char mask) 
{
	if(mask == 0) playersAccess.erase(peerId);
	else
	playersAccess.emplace(peerId, mask);
}

bool InventorySlot::playerAccessCheck(unsigned int peerId, unsigned char mask) 
{
	auto it = playersAccess.find(peerId);
	return it == playersAccess.cend() ? false : it->second &= mask;
}

void InventorySlot::sync() 
{
	if(TealEngine::Core::host.isLive()) 
	{
		InventorySlotDataPacket slotDataPacket;
		slotDataPacket.packetType = InventoryManagerPacketType::slotData;
		slotDataPacket.id = slotRegisteredId;
		std::vector<char> slotData = this->saveBuffer();
		std::vector<char> buffer(slotData.size() + sizeof(slotDataPacket));
		memcpy(buffer.data(), &slotDataPacket, sizeof(slotDataPacket));
		memcpy(buffer.data() + sizeof(InventorySlotDataPacket), slotData.data(), slotData.size());
		Logger::logInfo("Synchronizing slot start.");
		for(auto[peerId, accessMask] : playersAccess) 
		{
			if(accessMask &= InventorySlotAccessType::view) 
			{
				PeerInfo peer;
				peer.id = peerId;
				Logger::logInfo(std::format("Sending slot id {} data to peer id {}", slotRegisteredId, peerId));
				TealEngine::Core::host.send(World::currentWorld->inventoryManager->getNetworkId(), peer, buffer.data(), buffer.size());
			}
		}
		Logger::logInfo("Synchronizing slot end.");
	}
}

InventorySlot::~InventorySlot() 
{
	if(slotComponent) 
	{
		slotComponent->setSlot(nullptr);
	}
}