#include "PlayerBuilder.h"
#include "FollowNode.h"
#include "Inventory/InventoryItemTemplates.h"
#include "World.h"
#include "Inventory/InventorySlotComponent.h"
#include "Inventory/SubSlotsDisplayController.h"
#include "Entity/EntityTemplates.h"
#include "Entity/EntityTemplate.h"
#include "Crafting/PlayerCrafting.h"
#include "Crafting/Recipes.h"
#include "Crafting/RecipesDisplay.h"

Recipes playerRecipes;
bool playerRecipesInitialized = false;

void PlayerBuilder::build(PlayerType pt) 
{
	if(!playerRecipesInitialized) 
	{
		playerRecipesInitialized = true;
		playerRecipes.addRecipe(Recipe(
		{
			{"material.slate", 4}
		}, 
		{
			{"material.tiles", 1}
		}));

		playerRecipes.addRecipe(Recipe(
		{
			{"material.slate", 4}
		}, 
		{
			{"material.aztec0", 1}
		}));

		playerRecipes.addRecipe(Recipe(
		{
			{"material.slate", 4}, {"material.tiles", 4}
		}, 
		{
			{"material.aztec1", 1}
		}));
	}
	showInventoryUI = false;
	const int horisontalLD = 3;
	const int verticalLD = 2;
	playerType = pt;
	headNode = new GameNode3D();
	const float lookSensitivity = 0.005f;
	if(playerType == PlayerType::ControlledPlayer) 
	{
		this->getParent()->rename("MainPlayer");
		this->yaw = new LookControls();
		this->pitch = new LookControls();
		this->inventoryHotbar = new GameNodeGUI();
		this->inventoryBackpack = new GameNodeGUI();
		this->inventoryNode = new GameNodeGUI();
		inventoryHotbar->rename("InvHotbar");
		inventoryBackpack->rename("InvBackpack");
		inventoryNode->rename("Inventory");
		yaw->lookSensitivity.x = lookSensitivity;
		yaw->lookSensitivity.y = 0.0f;
		pitch->lookSensitivity.x = 0.0f;
		pitch->lookSensitivity.y = lookSensitivity;
		this->camera = new Camera();
		this->camera->setPlayPrimary(true);
		this->camera->setFixToWindowSize(true);
		this->camera->setPerspectiveProjection(90.0f, 1.0f, 0.01f, 3000.0f);
		this->movement = new PlayerMovement();
		this->movement->pb = this;
		Input::Mouse::setDisableCursorDefault(Input::Mouse::DefaultDisableCursorOption::DisableOnFrameStart);
		headNode->attachComponent(pitch);
		this->getParent()->attachComponent(yaw);

		//
		GameNode* absolute = new GameNode();
		GameNode3D* sky = new GameNode3D();
		absolute->rename("SkyAbsoluteTransformNode");
		sky->rename("SkyModel");
		Mesh3DRenderer* skyModel = new Mesh3DRenderer();
		FollowNode* follow = new FollowNode();
		follow->followNode = this->getParent();
		follow->positionFactor = 1.0f;
		follow->rotationFactor = 0.0f;
		sky->getRelativeTransform().setScale(glm::vec3(-2000.0f, 2000.0f, 2000.0f));
		skyModel->setMesh(&Core::modelsManager.get("uvsphere.obj"));
		skyModel->setMaterial(&Core::materialsManager.get("Sky"));
		sky->attachComponent(skyModel);
		sky->attachComponent(follow);
		absolute->addChild(sky);
		this->getParent()->addChild(absolute);

		this->worldHitscanDisplay = new WorldHitscanDisplay();
		this->headNode->attachComponent(worldHitscanDisplay);
	}
	else 
	{
		this->inventoryHotbar = new GameNode();
		this->inventoryBackpack = new GameNode();
		this->inventoryNode = new GameNode();
		inventoryHotbar->rename("InvHotbar");
		inventoryBackpack->rename("InvBackpack");
		inventoryNode->rename("Inventory");
	}
	if(playerType != PlayerType::ClientSidePlayer) 
	{
		this->inventory = new Inventory();
		this->chunkLoader = new ChunkLoader();
		this->chunkLoader->horisontalLoadDistance = horisontalLD;
		this->chunkLoader->verticalLoadDistance = verticalLD;
		this->getParent()->attachComponent(chunkLoader);
		this->crafting = new PlayerCrafting(&playerRecipes, inventory, this);
	}
	this->chunkLoader->updateRenderPosition = (playerType == PlayerType::ControlledPlayer);
	if(playerType != PlayerType::ClientSidePlayer) 
	{
		this->itemEquipper = new ItemEquipper();
		this->inventoryHotbar->attachComponent(itemEquipper);
		this->itemEquipper->setEquipNode(headNode);
		itemEquipper->mainPlayer = playerType == PlayerType::ControlledPlayer;
	}
	this->collider = new BoxCollider();
	this->body = new RigidBody();
	this->body->Material().bouncines = 0.05f;
	this->body->Material().friction = 0.025f;
	this->body->Material().mass = 1.0f;
	this->body->setFixedRotation(true);
	this->collider->from = glm::vec3(-0.15, -1.5, -0.15);
	this->collider->to = glm::vec3(0.15, 0.2, 0.15);
	this->getParent()->attachComponents({body, collider});
	

	this->getParent()->addChild(headNode);
	//UI
	if(playerType == PlayerType::ControlledPlayer) 
	{
		const float inventoryHorisontalPadding = 0.1f;
		((GameNodeGUI*)inventoryNode)->getTransform().setOffsetX(GUIUnit::windowWidths(inventoryHorisontalPadding));
		((GameNodeGUI*)inventoryNode)->getTransform().setWidth(GUIUnit::windowWidths(1.0f - inventoryHorisontalPadding * 2.0f));
		const float inventoryVetricalPadding = 0.0000000f;
		((GameNodeGUI*)inventoryNode)->getTransform().setOffsetY(GUIUnit::windowHeights(inventoryVetricalPadding));
		((GameNodeGUI*)inventoryNode)->getTransform().setHeight(GUIUnit::windowHeights(1.0f - inventoryVetricalPadding * 2.0f));
		
		const GUIUnit hotbarHeight = GUIUnit::pixels(72.0f);
		((GameNodeGUI*)inventoryHotbar)->getTransform().setOffsetX(GUIUnit::pixels(0));
		((GameNodeGUI*)inventoryHotbar)->getTransform().setOffsetY(GUIUnit::parentHeights(1.0f) - hotbarHeight);
		((GameNodeGUI*)inventoryHotbar)->getTransform().setWidth(GUIUnit::parentWidths(1.0f));
		((GameNodeGUI*)inventoryHotbar)->getTransform().setHeight(hotbarHeight);
		
		subSlots = new GameNode();
		subSlots->setActive(false);
		subSlots->rename("SubSlots");
		this->getParent()->addChild(subSlots);

		headNode->attachComponent(camera);
		headNode->rename("Head");

		craftingContainer = new GameNodeGUI();
		craftingContainer->rename("CraftingGUIContainer");
		craftingContainer->getTransform().setHeight(GUIUnit::windowHeights(0.8f));
		craftingContainer->getTransform().setWidth(GUIUnit::windowWidths(1.0f));
		craftingContainer->getTransform().setOffsetY(GUIUnit::windowHeights(0.1f));
		RecipesDisplay* recipesDisplay = new RecipesDisplay(&playerRecipes, inventory, this);
		craftingContainer->attachComponent(recipesDisplay);
		this->getParent()->addChild(craftingContainer);
	}

	for(int i = 0; i < hotbarSlotsCount; i++) 
	{
		GameNode* slotNode =  playerType == PlayerType::ControlledPlayer ? new GameNodeGUI() : new GameNode();
		InventorySlot* slot = new InventorySlot();
		InventorySlotComponent* slotComp = nullptr;
		if(playerType == PlayerType::ControlledPlayer) 
		{
			slotComp = new InventorySlotComponent();
			slotComp->setSlot(slot);
			slotNode->rename(std::format("slot[{}]", i));
			GUIUnit slotWidth = GUIUnit::parentHeights(1.0f);
			GUIUnit slotHotbarSpan = GUIUnit::parentWidths(1.0f) - slotWidth; 
			((GameNodeGUI*)slotNode)->getTransform().setOffsetX(slotHotbarSpan * ((float)i / ((float)hotbarSlotsCount - 1.0f)));
			((GameNodeGUI*)slotNode)->getTransform().setOffsetY(GUIUnit::pixels(0));
			((GameNodeGUI*)slotNode)->getTransform().setWidth(GUIUnit::parentHeights(1.0f));
			((GameNodeGUI*)slotNode)->getTransform().setHeight(GUIUnit::parentHeights(1.0f));
			((GameNodeGUI*)slotNode)->attachComponent(slotComp);
			GameNodeGUI* subSlotsNode = new GameNodeGUI();
			subSlotsNode->rename("SubSlotNode");
			SubSlotsDisplayController* subSlotsComponent = new SubSlotsDisplayController();
			subSlotsComponent->setObservedSlot(slotComp);
			subSlotsNode->attachComponent(subSlotsComponent);
			subSlots->addChild(subSlotsNode);
		}
		itemEquipper->addSlot(slot, slotComp);
		inventoryHotbar->addChild(slotNode);
		inventory->addSlot(slot);
		hotbarSlots[i] = slot;
	}

	inventoryNode->addChild(inventoryBackpack);
	inventoryNode->addChild(inventoryHotbar);
	this->getParent()->addChild(inventoryNode);
	this->getParent()->addChild(headNode);
	if(playerType == PlayerType::ControlledPlayer) 
	{
		this->getParent()->attachComponent(movement);
		World::currentWorld->playerBuilder = this;
	}
	
	if(playerType != PlayerType::ControlledPlayer) 
	{
		GameNode* models = GameNode::loadNodeFromJsonFile("./GameAssets/Player/PlayerModelContainer.node");
		this->getParent()->addChild(models);
	}
	inventory->pushStack(InventoryItem("mining_tool", 1));
	EntityHeader entityHeader;
	entityHeader.x = 0.0f;
	entityHeader.y = 0.0f;
	entityHeader.z = 0.0f;
	entityHeader.entityId = ~0;
	EntityTemplates::getInstance()->getTemplate("player")->createCommon(&entityHeader, sizeof(entityHeader), entityComponent, entityNode);
	this->getParent()->addChild(entityNode);
}

std::vector<char> PlayerBuilder::saveBuffer() 
{
	if(this->playerType == PlayerType::ClientSidePlayer) return {};
	PlayerSaveHeader header;
	Transform& t = this->getParentOfType<GameNode3D>()->getRelativeTransform();
	header.x = t.getPosition().x;
	header.y = t.getPosition().y;
	header.z = t.getPosition().z;
	header.dx = body->getVelocity().x;
	header.dy = body->getVelocity().y;
	header.dz = body->getVelocity().z;

	std::vector<char> buffer;
	std::vector<char> invBuffer = inventory->saveBuffer();
	header.dataSize = sizeof(header) + invBuffer.size();
	buffer.insert(buffer.end(), (char*)&header, (char*)&header + sizeof(header));
	buffer.insert(buffer.end(), invBuffer.begin(), invBuffer.end());
	return buffer;
}

void PlayerBuilder::loadBuffer(char* buffer) 
{
	if(this->playerType == PlayerType::ClientSidePlayer) return;
	PlayerSaveHeader header;
	header = *((PlayerSaveHeader*)buffer); 
	this->getParentOfType<GameNode3D>()->getRelativeTransform().setPosition(glm::vec3(header.x, header.y, header.z));
	this->body->setVelocity(glm::vec3(header.dx, header.dy, header.dz));
	this->inventory->loadBuffer(buffer + sizeof(header));
}

void PlayerBuilder::update() 
{
	if(this->showInventoryUI) 
	{
		Input::Mouse::setDisableCursorCurrent(false);
	}
}

void PlayerBuilder::onKeyPress(unsigned int key) 
{
	if(playerType == PlayerType::ControlledPlayer) 
	{
		if(key == GLFW_KEY_E) 
		{
			this->showInventoryUI = !this->showInventoryUI;
			subSlots->setActive(this->showInventoryUI);
			craftingContainer->setActive(this->showInventoryUI);
		}
	}
}

PlayerBuilder::~PlayerBuilder() 
{
	for(int i = 0; i < hotbarSlotsCount; i++) 
	{
		delete hotbarSlots[i];
	}
	delete this->inventory;
}