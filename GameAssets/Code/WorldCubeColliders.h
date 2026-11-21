#include "World.h"

using namespace TealEngine;

class WorldCubeColliders : public TilesCollider
{
	private:
	World* world;
	BoxCollider* boxCollider;
	public:
	virtual void onAttach() override;	
	virtual std::vector<Collider*> getCollidersAt(glm::vec3 position) override;
};