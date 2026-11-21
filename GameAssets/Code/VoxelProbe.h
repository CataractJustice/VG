#include "GameNode/Component.h"
using namespace TealEngine;

class VoxelProbe : public Component 
{
	private:
	public:
	VoxelProbe();
	float probeValue;
	void update() override;
};