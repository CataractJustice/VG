#include "GameNode/Component.h"
#include "GameNode/GameNode3D.h"
using namespace TealEngine;
class NodeTrail : public Component 
{
	private:
	struct Trail 
	{
		glm::vec3 position;
		glm::quat orientation;
		float lag;
	};
	float step;
	float distance;
	int segmentCount;
	std::string nodePath;
	std::vector<NodeTrail::Trail> trail;
	std::vector<GameNode3D*> segments;
	glm::vec3 oldPos;
	glm::quat oldQuat;
	public:
	NodeTrail();
	void onAttach();
	void update() override;
};