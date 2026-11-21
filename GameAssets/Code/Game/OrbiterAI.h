#include "GameNode/GameNode3D.h"
#include "GameNode/Component.h"
using namespace TealEngine;
class OrbiterAI : public Component
{
	private:
	GameNode* target;
	float speed;
	float turning;
	public:
	void update() override;
	OrbiterAI();
};