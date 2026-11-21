#pragma once
#include "GameNode/Component.h"
#include "LookControls.h"
#include "PlayerMovement.h"
#include "Physics/Collider.h"
#include "Physics/Bodies/RigidBody.h"
#include "Inventory/Inventory.h"
#include "ItemEquipper.h"
#include "ChunkLoader.h"
#include "GameNode/GameNodeGUI.h"
#include "WorldHitscanDisplay.h"
class PlayerCrafting;
class Entity;
class RecipesDisplay;
using namespace TealEngine;

struct PlayerSaveHeader 
{
	float x;
	float y;
	float z;
	float dx;
	float dy;
	float dz;
	unsigned int dataSize;
};

const unsigned int hotbarSlotsCount = 10;

class PlayerBuilder : public Component 
{
	private:
	public:
	BoxCollider* collider = nullptr;
	RigidBody* body = nullptr;
	LookControls* yaw = nullptr;
	LookControls* pitch = nullptr;
	Camera* camera = nullptr;
	GameNode3D* headNode = nullptr;
	Inventory* inventory = nullptr;
	PlayerMovement* movement = nullptr;
	ItemEquipper* itemEquipper = nullptr;
	GameNode* inventoryNode = nullptr;
	GameNode* inventoryHotbar = nullptr;
	GameNode* inventoryBackpack = nullptr;
	GameNodeGUI* craftingContainer;
	InventorySlot* hotbarSlots[hotbarSlotsCount];
	GameNode* subSlots = nullptr;
	ChunkLoader* chunkLoader = nullptr;
	WorldHitscanDisplay* worldHitscanDisplay = nullptr;
	GameNode3D* entityNode = nullptr;
	Entity* entityComponent = nullptr;
	PlayerCrafting* crafting;
	RecipesDisplay* recipesDisplay;
	bool showInventoryUI = false;
	enum class PlayerType 
	{
		ControlledPlayer,
		ClientSidePlayer,
		HostSidePlayer
	};
	PlayerType playerType;
	void build(PlayerType pt);
	std::vector<char> saveBuffer();
	void loadBuffer(char* buffer);
	void update() override;
	void onKeyPress(unsigned int key) override;
	~PlayerBuilder();
};