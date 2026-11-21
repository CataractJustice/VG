#pragma once
#include "GameNode/Component.h"
#include "Chunk.h"

using namespace TealEngine;

enum class EntityType 
{
	Player,
	ItemDrop,
	ItemPicker,
	LastType
};

class Entity : public Component 
{
	private:
	Chunk* chunk;
	unsigned int entityTemplateId;
	unsigned long long entityId;
	public:
	inline unsigned long long getEntityId() const { return entityId; };
	Entity(unsigned int templateId = 0);
	void update() override;
	inline const unsigned int getEntityTemplateId() const { return entityTemplateId; };
	inline Chunk* getChunk() const { return this->chunk; };
	void setChunk(Chunk* chunk);
	void onDestroy() override;
	std::vector<char> saveBuffer() const;
	std::vector<char> saveBufferCommon() const;
	void loadBuffer(const char* data, unsigned int size);
	void assignId(unsigned long long);
	void assignTemplateId(unsigned int);
};