#include "GameNode/Component.h"
#include "Graphics/Light/Light.h"
using namespace TealEngine;

class SunController : public Component 
{
	private:
	GameNode* player;
	DirectionLight* dl;
	glm::vec4 sunColor;
	glm::vec4 sunsetColor;
	float falloffHeightStart;
	float falloffLength;
	public:
	void update() override;
	SunController();
};