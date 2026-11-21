#include "PlayerMovement.h"
#include "GameNode/ComponentProp.h"
#include "GameNode/ComponentFactory.h"
#include "System/Input.h"
#include "GameNode/GameNode3D.h"
#include "libs/glm/gtc/type_ptr.hpp"
#include "Player/PlayerManager.h"
#include "Player/PlayerBuilder.h"
using namespace TealEngine;

PlayerMovement::PlayerMovement() 
{
	velocity = glm::vec3(0.0f);
	onGround = false;
	acceleration = 0.005f;
	jumpForce = 0.12f;
	maxAcceleratedVelocity = 0.04f;
	startVelocity = 0.020f;
	slowingDuringDirectionChangeThreshold = 0.95f;
	spaceHoldFlightUpForce = 0.0f;
	airDrag = 0.01f;
	airAcceleration = acceleration * 0.75f;
	jumpVelocityBoost = 0.01f;
	fullStopVelocity = 0.030f;
	this->body = nullptr;
	
	this->addProp(new FloatVecProp(glm::value_ptr(velocity), 3), "Velocity");
	this->addProp(new BoolProp(&onGround), "On ground");
	this->addProp(new FloatProp(&acceleration), "Acceleration");
	this->addProp(new FloatProp(&airAcceleration), "Air acceleration");


	this->addProp(new FloatProp(&jumpForce), "Jump force");
	this->addProp(new FloatProp(&maxAcceleratedVelocity), "Max accelerated velocity");
	this->addProp(new FloatProp(&startVelocity), "Move start velocity");
	this->addProp(new FloatProp(&fullStopVelocity), "Full stop velocity");
	this->addProp(new FloatProp(&slowingDuringDirectionChangeThreshold), "Direction change slowing threshold");
	this->addProp(new FloatProp(&spaceHoldFlightUpForce), "Space hold up force");
	this->addProp(new FloatProp(&airDrag), "Air drag");
}

void PlayerMovement::update() 
{
	if(!Component::isValidComponent(body) || !this->body) 
	{
		auto bodies = this->getParent()->findComponentsByType<RigidBody>();
		if(bodies.size()) 
		{
			this->body = bodies[0];
		}
		else 
		{
			return;
		}
	}
	//
	Transform& transform = ((GameNode3D*)this->getParent())->getRelativeTransform();
	//
	PlayerMovementStatePacket packet;
	packet.packetType = PlayerManagerPacketType::movement;
	packet.view = pb->headNode->getWorldTransform().forward();
	packet.position = transform.getPosition();
	packet.velocity = body->getVelocity();
	if(Core::client.isLive()) 
	{
		Core::client.send({Core::networkDictionary.getNetIdOf("PlayerManager"), 0}, &packet, sizeof(packet));
	}
	if(Core::host.isLive()) 
	{
		Core::host.broadcast({Core::networkDictionary.getNetIdOf("PlayerManager"), 0}, &packet, sizeof(packet));
	}
	
	bool moveKeyPressed = false;
	glm::vec3 bodyMoveVelocity = glm::vec3(this->body->getVelocity().x, 0.0f, this->body->getVelocity().z);
	float currentMoveVelocityMagnitute = glm::length(bodyMoveVelocity);

	//direction in which player wants to move
	glm::vec3 moveDir = glm::vec3(0.0f);

	if(
		Input::Keyboard::isKeyPressed(GLFW_KEY_W) ||
		Input::Keyboard::isKeyPressed(GLFW_KEY_A) || 
		Input::Keyboard::isKeyPressed(GLFW_KEY_S) ||
		Input::Keyboard::isKeyPressed(GLFW_KEY_D)
	)
	{
		moveKeyPressed = true;
	}

	if(!moveKeyPressed && !onGround) 
	{
		bodyMoveVelocity *= 1.0f - airDrag;
		body->setVelocity(glm::vec3(bodyMoveVelocity.x, body->getVelocity().y, bodyMoveVelocity.z));
	}
	//if we move slow enough we want to stop immediately so precise small step movement is possible
	if(onGround && currentMoveVelocityMagnitute < fullStopVelocity && !moveKeyPressed) 
	{
		body->setVelocity(glm::vec3(0.0f, glm::max(0.0f, body->getVelocity().y), 0.0f));
		body->useObjectLocalGravity = true;
	}
	else 
	{
		body->useObjectLocalGravity = false;
	}
	if(onGround) 
	{
		body->setVelocity(glm::vec3(body->getVelocity().x * 0.95f, body->getVelocity().y, body->getVelocity().z * 0.95f));
	}
	//
	if(Input::Keyboard::isKeyPressed(GLFW_KEY_W)) 
	{
		moveDir += transform.forward();
	}
	if(Input::Keyboard::isKeyPressed(GLFW_KEY_A)) 
	{
		moveDir -= transform.right();
	}
	if(Input::Keyboard::isKeyPressed(GLFW_KEY_S)) 
	{
		moveDir -= transform.forward();
	}
	if(Input::Keyboard::isKeyPressed(GLFW_KEY_D)) 
	{
		moveDir += transform.right();
	}
	
	if(moveKeyPressed && (moveDir.x || moveDir.y || moveDir.z)) 
	{
		moveDir = glm::normalize(moveDir);
		glm::vec3 accelerationVec = moveDir * (onGround ? acceleration : airAcceleration);
		glm::vec3 bodyMoveDir = glm::normalize(bodyMoveVelocity);
		float nextMoveVelocityMagnitute = glm::length(accelerationVec + bodyMoveVelocity);
		if(currentMoveVelocityMagnitute> maxAcceleratedVelocity && nextMoveVelocityMagnitute > currentMoveVelocityMagnitute) 
		{
			accelerationVec -= glm::dot(accelerationVec, bodyMoveDir) * bodyMoveDir;
		}
		bodyMoveVelocity += accelerationVec;
		body->setVelocity(glm::vec3(bodyMoveVelocity.x, body->getVelocity().y, bodyMoveVelocity.z));
	}

	if(Input::Keyboard::isKeyPressed(GLFW_KEY_SPACE)) 
	{
		this->body->addVelocity(glm::vec3(0.0f,spaceHoldFlightUpForce,0.0f));
	}

	onGround = false;
}

void PlayerMovement::onAttach() 
{
	auto bodies = this->getParent()->findComponentsByType<RigidBody>();
	if(bodies.size()) this->body = bodies[0];
}


void PlayerMovement::onCollision(const Collision& collision) 
{
	if(collision.relativePos.y < 0.0f && collision.normal.y > 0.5f) 
	{
		this->onGround = true;
		if(Input::Keyboard::isKeyPressed(GLFW_KEY_SPACE)) 
		{
			if(this->body) 
			{
				RigidBody* otherBody = nullptr;
				auto bodies = collision.collidedWith->getParent()->findComponentsByType<RigidBody>();
				if(bodies.size()) otherBody = bodies[0];
				float otherVelocity = otherBody ? otherBody->getVelocity().y : 0.0f;
				float thisVelocity = body->getVelocity().y;
				
				this->body->addVelocity(glm::vec3(0.0f, glm::max(0.0f, otherVelocity - thisVelocity + jumpForce), 0.0f));
			}
		}
	}
}

EXPORT_COMPONENT(PlayerMovement);