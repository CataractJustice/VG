#include "GameNode/Component.h"
using namespace TealEngine;
class MaterialDropRenderer : public Component
{
	public:
	void render(Kirino::StateColletion& state, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) override;
	static void init(unsigned int materialAtlas, glm::vec2 atlasScale);
	unsigned int materialId;
};