#include "InventorySlotComponent.h"
#include "Graphics/GUI/MonospaceBitmapText.h"
#include "Graphics/GUI/Background.h"
#include "Core.h"

InventoryItem InventorySlotComponent::heldItem;
void InventorySlotComponent::onClick(unsigned short button) 
{
	if(this->getSlotPtr() == nullptr) return;
	if(this->getSlot().getItem().amount == 0 && heldItem.amount == 0) return;
	if(this->getSlot().getItem().getTemplatePtr() != heldItem.getTemplatePtr()) 
	{
		InventoryItem slotOriginalItem = this->getSlot().getItem();
		this->getSlot().getItem().clear();
		if(heldItem.getTemplatePtr()) 
		{
			int pushed = this->getSlot().pushStack(heldItem);
			if(pushed != heldItem.amount) 
			{
				this->getSlot().getItem() = slotOriginalItem;
			}
			else 
			{
				heldItem = slotOriginalItem;
			}
		}
		else 
		{
			heldItem = slotOriginalItem;
		}
	}
	else 
	{
		heldItem.amount -= this->getSlot().pushStack(heldItem);
		if(heldItem.amount == 0) heldItem.clear();
	}
}

void InventorySlotComponent::update() 
{
	if(this->getSlotPtr() == nullptr) return;
	if(!this->getSlot().getItem().isEmpty()) 
	{
		if(!picturePresent) 
		{
			bgc->setTexture(getSlot().getItem().getTemplatePtr()->textureImage);
			picturePresent = true;
			currentPicture = getSlot().getItem().getTemplatePtr()->textureImage;
		}
		else 
		{
			//if(currentPicture != getSlot().getItem().getTemplatePtr()->textureImage) 
			{
				bgc->setTexture(getSlot().getItem().getTemplatePtr()->textureImage);
				currentPicture = getSlot().getItem().getTemplatePtr()->textureImage;
			}
		}
		text->setText(std::to_string(getSlot().getItem().amount));
	}
	else 
	{
		text->setText("");
		if(picturePresent) 
		{
			bgc->setTexture(&Core::textureManager.get("t.png"));
			picturePresent = false;
			currentPicture = 0;
		}
	}
}

void InventorySlotComponent::onAttach() 
{
	bgc->color = glm::vec4(0.1f, 0.2f, 0.15f, 0.50f);
	bgc->pressColor = glm::vec4(0.1f, 0.2f, 0.15f, 0.80f);
	this->getParent()->attachComponent(bgc);
	text->setColor(glm::vec4(1.0f));
	text->setSize(GUIUnit::pixels(11));
	text->setText("");
	text->setTexture(&Core::textureManager.get("11x11font.png"));
	this->getParent()->attachComponent(text);
}

InventorySlotComponent::InventorySlotComponent() 
{
	this->bgc = new Background();
	this->text = new MonospaceBitmapText();
	slotPtr =  nullptr;
}

void InventorySlotComponent::setSlot(InventorySlot* slot) 
{
	slotPtr = slot;
	if(slotPtr) slotPtr->setSlotComponent(this);
}

InventorySlotComponent::~InventorySlotComponent() 
{
}