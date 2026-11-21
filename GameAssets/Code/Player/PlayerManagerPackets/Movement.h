#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "libs/glm/glm.hpp"
#include "libs/glm/gtx/quaternion.hpp"
#pragma pack(push, 1)
struct PlayerMovementStatePacket 
{
	unsigned char packetType;
	int peerId;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 view;
};
#pragma pack(pop)