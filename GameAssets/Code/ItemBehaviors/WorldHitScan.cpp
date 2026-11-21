#include "WorldHitScan.h"
#include "GameNode/ComponentFactory.h"
#include "System/Input.h"
#include "Core.h"
#include "World.h"
#include "Graphics/Shapes/Line3D.h"

void WorldHitScan::update() 
{
	if(Input::Mouse::isButtonPressed(GLFW_MOUSE_BUTTON_1)) 
	{
		glm::vec3 hit = this->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition();
		glm::ivec3 hitIndex;
		glm::vec3 normal;
		bool hitSomething = World::currentWorld->rayCast(hit, this->getParentOfTypeConst<GameNode3D>()->getWorldTransform().forward(), 64, 64.0f, hitIndex, normal);
		const glm::vec4 hitColor = hitSomething ? glm::vec4(0.0f, 0.5f, 0.7f, 1.0f) : glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		const glm::vec4 normalColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		Core::shapesRenderer.pushShape(Line3D(hit - glm::vec3(1.0f, 0.0f, 0.0f) * 0.2f, hit + glm::vec3(1.0f, 0.0f, 0.0f) * 0.2f, hitColor));
		Core::shapesRenderer.pushShape(Line3D(hit - glm::vec3(0.0f, 1.0f, 0.0f) * 0.2f, hit + glm::vec3(0.0f, 1.0f, 0.0f) * 0.2f, hitColor));
		Core::shapesRenderer.pushShape(Line3D(hit - glm::vec3(0.0f, 0.0f, 1.0f) * 0.2f, hit + glm::vec3(0.0f, 0.0f, 1.0f) * 0.2f, hitColor));


		Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) - glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(1.0f, 0.0f, 0.0f), hitColor * 0.5f, 0.01f));
		Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) - glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(hitIndex) + glm::vec3(0.0f, 1.0f, 0.0f), hitColor * 0.5f, 0.01f));
		Core::shapesRenderer.pushShape(Line3D(glm::vec3(hitIndex) - glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(hitIndex) + glm::vec3(0.0f, 0.0f, 1.0f), hitColor * 0.5f, 0.01f));
		Core::shapesRenderer.pushShape(Line3D(hit, hit + normal * 2.0f, normalColor));
	}
}

EXPORT_COMPONENT(WorldHitScan);