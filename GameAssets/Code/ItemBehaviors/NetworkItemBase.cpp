#include "NetworkItemBase.h"
#include "Player/PlayerManagerPackets/Types.h"
#include "System/Input.h"
#include "Core.h"
#include "Player/PlayerManager.h"
#include "World.h"
//Custom packets recieved by host side item
void NetworkItemBase::processPacketHost(const void* data, unsigned int size) {};
//Custom packets recieved by client side item
void NetworkItemBase::processPacketClient(const void* data, unsigned int size) {};
//Methods overloaded by children
void NetworkItemBase::updateItem() {};
void NetworkItemBase::onUsePress(unsigned char usage) {};
void NetworkItemBase::onUseRelease(unsigned char usage) {};
void NetworkItemBase::onUseHoldTick(unsigned char usage) {};

NetworkItemBase::NetworkItemBase() 
{
	memset(heldUsages, 0, sizeof(heldUsages));
}

bool NetworkItemBase::shouldSendPacket() 
{
	return (item->mainPlayer && Core::client.isConnected()) || Core::host.isLive();
}

void NetworkItemBase::onAttach() 
{
	this->item = getParent()->findComponentsByType<EquippedItem>()[0];
	playerManagerNetId = {Core::networkDictionary.getNetIdOf("PlayerManager"), 0};
}

bool NetworkItemBase::isUsageHeld(unsigned char usage) 
{
	return heldUsages[usage];
}

void NetworkItemBase::onWindowMousePress(unsigned short button) 
{
	if(item->mainPlayer == false) return;
	unsigned char usage;
	if(button == GLFW_MOUSE_BUTTON_LEFT) usage = ItemUsageType::primary;
	else
	if(button == GLFW_MOUSE_BUTTON_RIGHT) usage = ItemUsageType::secondary;
	else return;
	doPress(usage);
}
void NetworkItemBase::onWindowMouseRelease(unsigned short button) 
{
	if(item->mainPlayer == false) return;
	unsigned char usage;
	if(button == GLFW_MOUSE_BUTTON_LEFT) usage = ItemUsageType::primary;
	else
	if(button == GLFW_MOUSE_BUTTON_RIGHT) usage = ItemUsageType::secondary;
	else return;
	doRelease(usage);
}
/*

doPress, doRelease and holdRefresh:
-called by main player both on client and host to send a usage packet
-called on other players on host side, after receiving a client packet, to invoke item use and to broadcast item use to other players

*/
void NetworkItemBase::doPress(unsigned char usageType) 
{
	//do nothing if item was not yet released before press
	if(isUsageHeld(usageType)) 
	{
		return;
	}
	onUsePress(usageType);
	heldUsages[usageType] = keepHoldingTicks;
	sync(usageType, ItemUpdateType::press, item->playerId);
}

void NetworkItemBase::doRelease(unsigned char usageType) 
{
	//do nothing if item was never pressed to be used before release
	if(!isUsageHeld(usageType)) 
	{
		return;
	}
	onUseRelease(usageType);
	heldUsages[usageType] = 0;
	sync(usageType, ItemUpdateType::release, item->playerId);
}
void NetworkItemBase::holdRefresh(unsigned char usage) 
{
	//If client sent a hold use without previously sending a press use ignore packet
	if(Core::host.isLive() && heldUsages[usage] == 0) 
	{
		return;
	}
	heldUsages[usage] = keepHoldingTicks;
	sync(usage, ItemUpdateType::hold, item->playerId);
}

void NetworkItemBase::update() 
{
	//Do the item update
	updateItem();
	for(int i = 0; i < (unsigned char)ItemUsageType::maxUsageType; i++) 
	{
		if(heldUsages[i] > 0) 
		{
			//if this is controlled player send hold refresh packet once in a while
			if(item->mainPlayer && heldUsages[i] == holdingTicksRefreshAt) 
			{
				holdRefresh(i);
			}
			//Do the item hold update
			onUseHoldTick(i);
			heldUsages[i]--;
			//Player stopped sending hold refresh packets and has not sent a release packet
			if(heldUsages[i] == 0 && item->mainPlayer == false && Core::host.isLive()) 
			{
				//force release action
				onUseRelease(i);
				sync(i, ItemUpdateType::release, item->playerId);
			}
		}
	}
}
/*
If this is a multiplayer, and not a client side other player
-create item use packet
If this is a client main player
-send usage packet
If this is a host
-send usage packet to all players in range of the using player, this will woth both for host-player and client-players on the host side
*/
void NetworkItemBase::sync(unsigned char usage, unsigned char update, unsigned int peerId) 
{
	if(shouldSendPacket()) 
	{
		PlayerUseItemPacket packet;
		packet.packetType = PlayerManagerPacketType::useItem;
		packet.hotbarIndex = item->slotIndex;
		//ignored by host receiving from client
		packet.peerId = peerId;
		packet.updateType = update;
		packet.usageType = usage;
		if(Core::client.isConnected()) 
		{
			Core::client.send(playerManagerNetId, &packet, sizeof(packet));
		}
		else 
		{
			const auto inRange = World::currentWorld->playerManager->getInRange(peerId, itemUsageSyncRange);
			for(const auto&[id, player] : inRange) 
			{
				if(id == PlayerManager::hostPlayerId) continue;
				PeerInfo peer;
				peer.id = id;
				Core::host.send(playerManagerNetId, peer, &packet, sizeof(packet));
			}
		}
	}
}
