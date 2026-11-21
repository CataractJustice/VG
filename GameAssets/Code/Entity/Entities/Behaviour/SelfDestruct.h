#include "GameNode/Component.h"

using namespace TealEngine;

class SelfDestruct : public Component 
{
	private:
	public:
	unsigned int ticksLeft;
	SelfDestruct(unsigned int updateTicksBeforeDestoying = 0);
	void update() override;
};