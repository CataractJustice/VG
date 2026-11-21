#pragma once
#include "GameNode/Component.h"
#include "Physics/Collision.h"
#include "Physics/Bodies/RigidBody.h"
using namespace TealEngine;
class PlayerBuilder;
class PlayerMovement : public TealEngine::Component 
{
	private:
	glm::vec3 velocity;
	TealEngine::RigidBody* body;
	bool onGround;
	float jumpForce;
	float acceleration;
	float airAcceleration;
	float maxAcceleratedVelocity;
	float startVelocity;
	float fullStopVelocity;
	float slowingDuringDirectionChangeThreshold;
	float spaceHoldFlightUpForce;
	float jumpVelocityBoost;
	float airDrag;
	public:
	PlayerBuilder* pb;
	void update() override;
	PlayerMovement();
	void onAttach() override;
	void onCollision(const Collision& collision) override;
};