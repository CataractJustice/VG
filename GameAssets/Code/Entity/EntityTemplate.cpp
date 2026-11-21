#include "EntityTemplate.h"
#include "Entity/Entity.h"

void EntityTemplate::loadBuffer(const char* data, unsigned int size, Entity* entity) const
{

}

std::vector<char> EntityTemplate::saveBufferCommon(const Entity* entity) const
{
	glm::vec3 position = entity->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition();
	EntityHeader header;
	header.entityId = entity->getEntityId();
	header.entityTemplateId = this->assignedId;
	header.x = position.x;
	header.y = position.y;
	header.z = position.z;
	std::vector<char> entityData = entity->saveBuffer();
	std::vector<char> buffer(sizeof(header) + entityData.size());
	memcpy(buffer.data(), &header, sizeof(header));
	memcpy(buffer.data() + sizeof(header), entityData.data(), entityData.size());
	return buffer;
}

void EntityTemplate::loadBufferCommon(const char* data, unsigned int size, Entity* entity) const
{
	const EntityHeader& header = *(const EntityHeader*)data;
	entity->getParentOfType<GameNode3D>()->getRelativeTransform().setPosition(glm::vec3(header.x, header.y, header.z));
	loadBuffer(data + sizeof(EntityHeader), size - sizeof(EntityHeader), entity);
}

bool EntityTemplate::createCommon(const void* data, unsigned int size, Entity*& entityOut, GameNode3D*& nodeOut) const
{
	if(size < sizeof(EntityHeader)) return false;
	const EntityHeader& header = *(const EntityHeader*)data;
	nodeOut = new GameNode3D();
	entityOut = new Entity(this->assignedId);
	nodeOut->attachComponent(entityOut);
	nodeOut->getRelativeTransform().setPosition(glm::vec3(header.x, header.y, header.z));
	return this->create(((const char*)data) + sizeof(EntityHeader), size - sizeof(EntityHeader), entityOut, nodeOut);
}