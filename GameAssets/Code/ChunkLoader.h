#pragma once
#include "GameNode/Component.h"

using namespace TealEngine;

class ChunkLoader : public Component 
{
	void update() override;
	public:
	int horisontalLoadDistance;
	int verticalLoadDistance;
	int counter;
	bool updateRenderPosition;
	ChunkLoader();
};