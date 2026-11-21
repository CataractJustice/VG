#pragma once
#include <unordered_map>
#include <string>
#include <vector>
class EntityTemplate;
class EntityTemplates 
{
	private:
	std::unordered_map<std::string, unsigned int> templatesMap;
	std::vector<EntityTemplate*> templates;
	static EntityTemplates* instance;
	public:
	static inline EntityTemplates* getInstance() 
	{
		if(!instance) instance = new EntityTemplates();
		return instance;
	};
	EntityTemplates();
	void registerEntityTemplate(const std::string& name, EntityTemplate* entityTemplate);
	const EntityTemplate* getTemplate(const std::string& name) const;
	const EntityTemplate* getTemplate(unsigned int id) const;
	const unsigned int getTemplateId(const std::string& name) const;
	bool createEntityFromBuffer(const void* data, unsigned int size, Entity*& entityOut, GameNode3D*& nodeOut);
};