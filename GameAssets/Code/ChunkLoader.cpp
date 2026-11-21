#include "ChunkLoader.h"
#include "GameNode/ComponentFactory.h"
#include "GameNode/Properties/GameNodeProp.h"
#include "World.h"
void ChunkLoader::update() 
{
	glm::ivec3 originChunk = vec3ToIndex(this->getParentOfType<GameNode3D>()->getWorldTransform().getPosition(), World::currentWorld->getChunkSize());
	glm::ivec3 loadVolume = glm::ivec3(horisontalLoadDistance, verticalLoadDistance, horisontalLoadDistance);
	World::currentWorld->loadArea(originChunk - loadVolume, originChunk + loadVolume, 100);
	if(updateRenderPosition) World::currentWorld->setRenderPosition(originChunk);
}

ChunkLoader::ChunkLoader() 
{
	counter = 0;
	horisontalLoadDistance = 5;
	verticalLoadDistance = 2;
	this->addProp(new IntProp(&horisontalLoadDistance), "Horisontal load distance");
	this->addProp(new IntProp(&verticalLoadDistance), "Vertical load distance");
}

EXPORT_COMPONENT(ChunkLoader);