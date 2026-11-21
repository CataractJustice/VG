#include "Entity.h"
#include "World.h"
#include "EntityTemplates.h"
#include "EntityTemplate.h"

Entity::Entity(unsigned int templateId) : entityTemplateId(templateId), chunk(nullptr)
{
	entityId = ~0ULL;
}

void Entity::update() 
{
	glm::ivec3 newIndex = vec3ToIndex(this->getParentOfType<GameNode3D>()->getWorldTransform().getPosition(), World::currentWorld->getChunkSize());
	Chunk* newChunk = World::currentWorld->getChunk(newIndex);
	if(chunk == newChunk) return;
	this->setChunk(newChunk);
}

void Entity::setChunk(Chunk* newChunk) 
{
	if(newChunk == chunk) return;
	if(this->chunk) 
	{
		this->chunk->entities[this->entityTemplateId].erase(this);
	}
	this->chunk = newChunk;
	if(this->chunk) 
	{
		this->chunk->entities[this->entityTemplateId].emplace(this);
	}
};

void Entity::onDestroy() 
{
	this->setChunk(nullptr);
	if(Core::host.isLive()) 
	{
		World::currentWorld->entityManager->deregisterEntity(this);
	}
}

std::vector<char> Entity::saveBuffer() const
{
	return EntityTemplates::getInstance()->getTemplate(entityTemplateId)->saveBuffer(this);
}

std::vector<char> Entity::saveBufferCommon() const
{
	return EntityTemplates::getInstance()->getTemplate(entityTemplateId)->saveBufferCommon(this);
}

void Entity::loadBuffer(const char* data, unsigned int size) 
{
	EntityTemplates::getInstance()->getTemplate(entityTemplateId)->loadBufferCommon(data, size, this);
}

void Entity::assignId(unsigned long long id) 
{
	if(this->entityId != ~0ULL) 
	{
		throw std::runtime_error("Entity id assigned twice");
	}
	this->entityId = id;
}

void Entity::assignTemplateId(unsigned int id) 
{
	if(this->entityTemplateId != ~0U) 
	{
		throw std::runtime_error("Entity template id assigned twice");
	}
	this->entityTemplateId = id;
}