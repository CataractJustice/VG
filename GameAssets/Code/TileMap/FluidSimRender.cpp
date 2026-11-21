#include "FluidSimRender.h"
#include "Core.h"
void FluidSimRender::postRender(Kirino::StateColletion& stateCollection, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) 
{
	return;
	fluidSim->render(this->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition(), stateCollection, frustum, renderTextures);
}

void FluidSimRender::shadowRender(Kirino::StateColletion& stateCollection, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) 
{
	return;
}