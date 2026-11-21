#include "VoxelProbe.h"
#include "GameNode/ComponentFactory.h"
#include "Core.h"

VoxelProbe::VoxelProbe() 
{
	this->addProp(new FloatProp(&probeValue), "Value");
}

void VoxelProbe::update() 
{
	glm::vec3 pos = this->getParentOfType<GameNode3D>()->getWorldTransform().getPosition();
	//float val = VoxelWorld::instance->getScalarFieldValueAt(pos);
	//probeValue = val;
	//Core::shapesRenderer.pushShape(Line3D(pos-glm::vec3(0.3f), pos+glm::vec3(0.3f), val > 0.0f ? glm::vec4(0.0f, 0.5f, 0.2f, 1.0f) : glm::vec4(1.0f)));
}

EXPORT_COMPONENT(VoxelProbe);