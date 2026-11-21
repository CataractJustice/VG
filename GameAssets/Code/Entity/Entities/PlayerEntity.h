#pragma once
#include "Entity/EntityTemplate.h"
class Entity;
class PlayerBuilder;
class PlayerEntity : public EntityTemplate 
{
	public:
	bool create(const void* data, unsigned int size, Entity* entity, GameNode3D* node) const  override;
	virtual std::vector<char> saveBuffer(const Entity* entity) const override;
	static PlayerBuilder* getPlayerBuilder(Entity* playerEntity);
};
