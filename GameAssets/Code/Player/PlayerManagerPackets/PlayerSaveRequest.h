#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "libs/glm/glm.hpp"
#include "libs/glm/gtx/quaternion.hpp"
#pragma pack(push, 1)
struct PlayerSaveRequestPacket 
{
	unsigned char packetType;
};
#pragma pack(pop)