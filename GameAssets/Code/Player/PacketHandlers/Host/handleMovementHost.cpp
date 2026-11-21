#include "Player/PlayerManager.h"
#include "System/Logger.h"
void PlayerManager::handleMovementHost(PeerInfo& peer, const void* data, unsigned int size)
{
	if(size != sizeof(PlayerMovementStatePacket)) 
	{
		Logger::logInfo(std::format("Invalid player movement packet size from peer id {}, received data size is: {}, required packet size is: {}.", peer.id, size, sizeof(PlayerMovementStatePacket)));
		return;
	}
	auto playerIt = players.find(peer.id);
	if(playerIt == players.cend()) 
	{
		Logger::logInfo(std::format("Peer id {} sent movement packet before logging in.", peer.id));
		return;
	}
	auto&[id, player] = *playerIt;
	const PlayerMovementStatePacket& packet = *(PlayerMovementStatePacket*)data;
	const float viewDirectionLimit = 1.01f;
	const float velocityLimit = 1.5f;
	const float positionLimit = 100000000.0f;
	const glm::vec3 positionDeltaLimit = glm::abs(packet.velocity) * 10000.0f + glm::vec3(1.0f);
	const float velocityDeltaLimit = 10000.2f;
	const glm::vec3 currentPosition = player.node->getRelativeTransform().getPosition();
	const glm::vec3 currentVelocity = player.builder->body->getVelocity();
	for(int axis = 0; axis < 3; axis++) 
	{
		const float position = packet.position[axis];
		const float velocity = packet.velocity[axis];
		const float view = packet.view[axis];
		// Reject NaN / +/- infinity.
		if(!std::isfinite(position) ||
			!std::isfinite(velocity) ||
			!std::isfinite(view)
		)
		{
					return;
		}
		// Absolute sanity limits.
		if(glm::abs(position) > positionLimit || 
			glm::abs(velocity) > velocityLimit || 
			glm::abs(view) > viewDirectionLimit
		)
		{
			return;
		}
		// Position cannot move farther than the reported velocity allows.
		if(glm::abs(position - currentPosition[axis]) > positionDeltaLimit[axis])
		{
			return;
		}
		// Velocity cannot change too abruptly.
		if(glm::abs(velocity - currentVelocity[axis]) > velocityDeltaLimit)
		{
			return;
		}
	}
	player.builder->body->setVelocity(packet.velocity);
	player.node->getRelativeTransform().setPosition(packet.position);
	glm::vec3 hLook = glm::vec3(packet.view.x, 0.0f, packet.view.z);
	glm::vec3 vLook = glm::vec3(0.0f, packet.view.y, glm::length(hLook));
	player.node->getRelativeTransform().lookAt(hLook, glm::vec3(0.0f, 1.0f, 0.0f));
	player.builder->headNode->getRelativeTransform().lookAt(vLook, glm::vec3(0.0f, 1.0f, 0.0f));
}