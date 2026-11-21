#include "Physics/Colliders/ScalarFieldCollider.h"
#include "World.h"
using namespace TealEngine;
class VoxelWorldCollider : public ScalarFieldCollider 
{
	private:
	World* world;
	public:
	float getScalarFieldAt(glm::vec3 position) override;
	glm::vec3 getNormalAt(glm::vec3 position) override;
	void onAttach() override;
};