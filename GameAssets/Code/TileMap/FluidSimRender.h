#pragma once
#include "FluidSim.h"
#include "GameNode/Component.h"
using namespace TealEngine;
class FluidSimRender : public Component 
{
	public:
	FluidSim* fluidSim;
	void postRender(Kirino::StateColletion& stateCollection, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) override;
	void shadowRender(Kirino::StateColletion& stateCollection, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) override;
};