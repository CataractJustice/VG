#pragma once
#include "GameNode/Component.h"
using namespace TealEngine;
class EntityNetworkListener : public Component
{
	public:
	virtual void onClientEntityPacket(const void* data, unsigned int size) = 0;
};