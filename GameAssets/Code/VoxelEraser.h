#include "GameNode/Component.h"
using namespace TealEngine;
class VoxelEraser : public Component 
{
	private:
	float radius;
	public:
	VoxelEraser();
	void update() override;
};