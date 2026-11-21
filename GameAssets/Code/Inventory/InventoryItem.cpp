#include "InventoryItem.h"
#include "InventorySlot.h"
InventoryItem::InventoryItem() : amount(0), templatePtr(nullptr)
{
}

InventoryItem::InventoryItem(ItemTemplateId id, int amount) 
{
	init(id, amount);
}
void InventoryItem::init(ItemTemplateId id, int amount) 
{
	this->templateId = id;
	this->amount = amount;
	this->templatePtr = InventoryItemTemplates::getInstance().getById(this->templateId);
	this->slots.clear();
	if(templatePtr)
	{
		for(const SlotTemplate& slotTemplate : templatePtr->slots) 
		{
			this->slots.push_back(std::make_shared<InventorySlot>());
			this->slots.back().get()->useWhiteList = slotTemplate.useWhiteList;
			this->slots.back().get()->whiteList = slotTemplate.whiteList;
			this->slots.back().get()->blackList = slotTemplate.blackList;
		}
	}	
}
const InventoryItemTemplate* InventoryItem::getTemplatePtr() const
{
	if(this->isEmpty()) return nullptr;
	return templatePtr;
}

bool InventoryItem::isEmpty() const 
{
	return amount == 0 || templateId == "" || templatePtr == nullptr;
}

const ItemTemplateId& InventoryItem::getId() const 
{
	return this->templateId;
}

void InventoryItem::clear() 
{
	this->amount = 0;
	this->templateId = "";
	this->templatePtr = nullptr;
	this->slots.clear();
}

std::vector<char> InventoryItem::saveBuffer() const
{
	std::vector<char> buff;
	buff.resize(sizeof(int) + this->getId().length() + 1);
	*((int*)buff.data()) = this->amount;
	memcpy(buff.data() + sizeof(int), this->getId().c_str(), this->getId().length() + 1);
	return buff;
}

void InventoryItem::loadBuffer(const char* buffer)
{
	this->init(buffer + sizeof(int), *(int*)buffer);
}

bool InventoryItem::loadBufferSafe(const char* buffer, unsigned int size)
{
	if(size <= sizeof(int)) return false;

	const int maxLen = size - sizeof(int);
	size_t len = strnlen(buffer + sizeof(int), maxLen);

	if(len >= maxLen) return false;
	if(!InventoryItemTemplates::getInstance().getById(buffer + sizeof(int))) return false;

	loadBuffer(buffer);
	return true;
}
