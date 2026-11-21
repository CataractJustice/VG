#pragma once
#include <vector>
#include "GameNode/GameNode3D.h"
using namespace TealEngine;
class Entity;
const unsigned int entityTemplateNameSize = 128;

#pragma pack(push, 1)
struct EntityHeader 
{
	unsigned int entityTemplateId;
	unsigned long long entityId;
	float x;
	float y;
	float z;
};
#pragma pack(pop)
class EntityTemplate
{
	protected:
	unsigned int assignedId = ~0U;
	public:
	inline void assignId(unsigned int id) 
	{
		if(assignedId != ~0U) throw std::runtime_error("Can't re-assign id to a template");
		assignedId = id;
	}
	virtual bool create(const void* data, unsigned int size, Entity* entityOut, GameNode3D* nodeOut) const  = 0;
	virtual std::vector<char> saveBuffer(const Entity* entity) const = 0;
	virtual void loadBuffer(const char* data, unsigned int size, Entity* entity) const;
	bool createCommon(const void* data, unsigned int size, Entity*& entityOut, GameNode3D*& nodeOut) const;
	std::vector<char> saveBufferCommon(const Entity* entity) const;
	void loadBufferCommon(const char* data, unsigned int size, Entity* entity) const;
};