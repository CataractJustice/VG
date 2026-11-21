#pragma once
#include "GameNode/Component.h"
#include "GameNode/GameNode3D.h"
#include "World.h"
#include "Components/InverseKinematics/InverseKinematicComponent.h"
using namespace TealEngine;

class GrabberArm : public Component
{
	public:
	std::vector<GameNode3D*> armNodes;
	glm::vec3 armTarget;
	glm::vec3 oldArmTarget;
	glm::vec3 armTargetInterpolated;
	float interpolationFactor;
	float interpolationSpeed;
	float radius;
	float stepRadius;
	float stepHeight;
	glm::vec3 castOffset;
	InverseKinematics invKinematics;
	void onAttach() override;
	void update() override;
	GrabberArm();
};