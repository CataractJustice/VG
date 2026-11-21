#include "VoxelEraser.h"
#include "GameNode/ComponentProp.h"
#include "System/Input.h"
#include "GameNode/ComponentFactory.h"
#include "Core.h"
#include "VoxelData.h"
VoxelEraser::VoxelEraser() 
{
	this->addProp(new FloatProp(&this->radius), "radius");
}

void VoxelEraser::update() 
{
	if(Input::Mouse::isButtonPressed(GLFW_MOUSE_BUTTON_1) || Input::Mouse::isButtonPressed(GLFW_MOUSE_BUTTON_2)) 
	{
		const Transform& t = this->getParentOfType<GameNode3D>()->getWorldTransform();
		glm::vec3 p = t.getPosition();
		glm::vec3 f = t.forward();
		const float step = 0.5f;
		float l;
		bool hit = false;
		for(l = 0.0f; l <= 200.0f; l += step) 
		{
			{
				hit = true;
				break;
			}
		}

		if(hit)
		{
			for(float i = -radius; i <= radius; i += 1.0f) 
			{
				for(float j = -radius; j <= radius; j += 1.0f) 
				{
					for(float k = -radius; k <= radius; k += 1.0f)
					{
						float falloff = (radius - glm::length(glm::vec3(i,j,k))) / radius;
						short va = 0;
						falloff += (float)(va) / voxelAmountScale / radius;
						if(falloff < 0.0f) continue;
						if(Input::Mouse::isButtonPressed(GLFW_MOUSE_BUTTON_1)) 
						{
							//VoxelWorld::instance->modifyVoxel(p + f * l + glm::vec3(i,j,k), {0, (short)(-1600.0f * falloff)}, (int)(VoxelModifyMethod::ADD_VOXEL) | (int)(VoxelModifyMethod::IF_NOT_EMPTY) | (int)(VoxelModifyMethod::IF_EXPOSED) | (int)(VoxelModifyMethod::KEEP_TYPE));
						} 
						else 
						{
							/*
							if(falloff < 0.5f) 
							{
								VoxelWorld::instance->modifyVoxel(p + f * l + glm::vec3(i,j,k), {0, -32000}, (int)(VoxelModifyMethod::SET_VOXEL) | (int)(VoxelModifyMethod::IF_EMPTY));
							}
							else 
							{
								VoxelWorld::instance->modifyVoxel(p + f * l + glm::vec3(i,j,k), {0, 1}, (int)(VoxelModifyMethod::SET_VOXEL) | (int)(VoxelModifyMethod::IF_EMPTY));
							}
							*/
						}
					}
				}
			}

			//Core::shapesRenderer.pushShape(Line3D(p+f*l - glm::vec3(1.0f, 1.0f, 1.0f), p+f*l + glm::vec3(1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.1f));
			//Core::shapesRenderer.pushShape(Line3D(p+f*l - glm::vec3(-1.0f, 1.0f, 1.0f), p+f*l + glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.1f));
			//Core::shapesRenderer.pushShape(Line3D(p+f*l - glm::vec3(1.0f, -1.0f, 1.0f), p+f*l + glm::vec3(1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.1f));
			//Core::shapesRenderer.pushShape(Line3D(p+f*l - glm::vec3(-1.0f, -1.0f, 1.0f), p+f*l + glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.1f));
		}
	}
}

EXPORT_COMPONENT(VoxelEraser);
