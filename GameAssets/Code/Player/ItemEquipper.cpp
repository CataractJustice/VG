#include "ItemEquipper.h"
#include "System/Input.h"
#include "GameNode/GameNode3D.h"
#include "GameNode/ComponentFactory.h"
#include "Graphics/GUI/Background.h"
#include "EquippedItem.h"
#include "Core.h"
#include "System/Logger.h"
#include "PlayerManager.h"

ItemEquipper::ItemEquipper() : currentItemObject(0), selectedItemIndex(-1), playerId(20050916)
{
	
}

void ItemEquipper::updateItemObject() 
{
	//No items left in current stack
	if(getEquipedItemStack().isEmpty() && currentItemObject) 
	{
		currentItemObject->destroy();
		this->currentItemObject = nullptr;
	}
	//Another item has been equipped
	if(!getEquipedItemStack().isEmpty() && currentItemObject == nullptr) 
	{
		std::string newItemPath;
		if(!mainPlayer && getEquipedItemStack().getTemplatePtr()->onOtherEquipNodePath.length())
		{
			newItemPath = getEquipedItemStack().getTemplatePtr()->onOtherEquipNodePath;
		}
		else 
		{
			newItemPath = getEquipedItemStack().getTemplatePtr()->onEquipNodePath;
		}
		if(newItemPath.length() == 0) 
		{
			Logger::logInfo(std::format("ItemEquipper: Item id {} has no item node path.", getEquipedItemStack().getId()));
			return;
		}
		this->currentItemObject = GameNode::loadNodeFromJsonFile(newItemPath);
		this->currentItemObject->attachComponent(new EquippedItem(&getEquipedItemStack(), slots[selectedItemIndex], mainPlayer, selectedItemIndex, playerId));
		equipNode->addChild(this->currentItemObject);
	}
}

void ItemEquipper::selectItemSlot(int index) 
{
	const glm::vec4 bgColor = glm::vec4(0.4f, 0.4f, 0.4f, 0.5f);
	const glm::vec4 bgSelectedColor = glm::vec4(0.7f, 0.7f, 0.7f, 0.8f);
	if(slots.size() == 0) return;
	index += this->slots.size();
	index %= this->slots.size();
	if(this->selectedItemIndex == index) return;
	if(mainPlayer && selectedItemIndex < slotDisplays.size()) 
	{
		//set old slot color to unselected
		auto bgsold = slotDisplays[selectedItemIndex]->getParent()->findComponentsByType<Background>();
		if(bgsold.size()) bgsold[0]->setColor(bgColor);
		//set new slot color to selected
		auto bgs = slotDisplays[index]->getParent()->findComponentsByType<Background>();
		if(bgs.size()) bgs[0]->setColor(bgSelectedColor);
	}
	
	selectedItemIndex = index;
	//delete old item
	if(currentItemObject) 
	{
		this->currentItemObject->destroy();
		this->currentItemObject = nullptr;
	}
	//load new item if stack is not empty
	if(!getEquipedItemStack().isEmpty()) 
	{
		updateItemObject();
	}

	if(mainPlayer && Core::client.isConnected()) 
	{		
		PlayerEquippSlotPacket equippPacket;
		equippPacket.packetType = PlayerManagerPacketType::equippSlot;
		equippPacket.slot = selectedItemIndex;
		Core::client.send({Core::networkDictionary.getNetIdOf("PlayerManager"), 0}, &equippPacket, sizeof(equippPacket));
	}
}

InventoryItem& ItemEquipper::getEquipedItemStack() 
{
	return this->slots[this->selectedItemIndex]->getItem();
}

void ItemEquipper::addSlot(InventorySlot* slot, InventorySlotComponent* display) 
{
	if(slotDisplays.size() && display == nullptr) 
	{
		throw std::invalid_argument("Mixed ItemEquipper slot types");
	}
	if(display) 
	{
		this->slotDisplays.push_back(display);
	}
	this->slots.push_back(slot);
}

void ItemEquipper::update() 
{
	if(slots.size() == 0) return;
	if(mainPlayer) 
	{
		selectItemSlot(selectedItemIndex + static_cast<int>(Input::Mouse::getDeltaScrollPos()));
		updateItemObject();
	}
}

EXPORT_COMPONENT(ItemEquipper);