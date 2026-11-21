#include "EntityTemplate.h"
#include "EntityTemplates.h"
#include "Entities/ItemDrop.h"
#include "Entities/PlayerEntity.h"
#include "EntityTemplate.h"
#include <format>

EntityTemplates* EntityTemplates::instance = nullptr;

EntityTemplates::EntityTemplates() 
{
	registerEntityTemplate("item", new ItemDrop());
	registerEntityTemplate("player", new PlayerEntity());
}
void EntityTemplates::registerEntityTemplate(const std::string& name, EntityTemplate* entityTemplate) 
{
	entityTemplate->assignId(templates.size());
	this->templatesMap[name] = templates.size();
	this->templates.push_back(entityTemplate);
}
const EntityTemplate* EntityTemplates::getTemplate(const std::string& name) const 
{
	auto it = templatesMap.find(name);
	if(it == templatesMap.cend()) 
	{
		throw std::invalid_argument(std::format("Can't find entity template with \"{}\" name.", name));
		return nullptr;
	}
	return templates[it->second];
}
const EntityTemplate* EntityTemplates::getTemplate(unsigned int id) const 
{
	return templates[id];
}

const unsigned int EntityTemplates::getTemplateId(const std::string& name) const 
{
	auto it = templatesMap.find(name);
	if(it == templatesMap.cend()) 
	{
		throw std::invalid_argument(std::format("Can't find entity template with \"{}\" name.", name));
		return 0;
	}
	return it->second;
};

bool EntityTemplates::createEntityFromBuffer(const void* data, unsigned int size, Entity*& entityOut, GameNode3D*& nodeOut) 
{
	if(size < sizeof(EntityHeader)) 
	{
		return false;
	}
	const EntityHeader& header = *(const EntityHeader*)data;
	if(header.entityTemplateId >= this->templates.size()) return false;
	EntityTemplate* entityTemplate = templates[header.entityTemplateId];
	if(!entityTemplate->createCommon(data, size, entityOut, nodeOut)) 
	{
		throw std::runtime_error("Failed to create entity");
		return false;
	}
	return true;
}