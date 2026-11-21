#include "PlayerCrafting.h"
#include "Recipes.h"
#include "Inventory/Inventory.h"
#include "World.h"
#include "Entity/Entities/ItemDrop.h"
#include "Player/PlayerBuilder.h"
#include "Entity/EntityTemplates.h"
#include "Core.h"
PlayerCrafting::PlayerCrafting(const Recipes* recipes, Inventory* inv, const PlayerBuilder* player) : 
recipes(recipes), 
inventory(inv), 
player(player)
{
}

//This must be server side
void PlayerCrafting::tryCraftByRecipeIndex(unsigned int index) 
{
	if(index >= recipes->getRecipes().size()) return;
	const Recipe& recipe = recipes->getRecipes()[index];
	if(!recipe.checkRequirements(inventory->getItemCountsMap())) return;
	if(Core::client.isConnected() == false) 
	{
		inventory->removeItemsByIdAndCount(recipe.getRequiredItems());
		std::map<std::string, unsigned int> couldNotFit;
		inventory->addItemsByIdAndCount(recipe.getOutputItems(), couldNotFit);
		for(const auto&[id, count] : couldNotFit) 
		{
			InventoryItem item;
			item.init(id, count);
			float vx = (((float)(rand() % 1024) / 1024.0f) * 2.0f - 1.0f) * 0.1f;
			float vy = (((float)(rand() % 1024) / 1024.0f)) * 0.1f;
			float vz = (((float)(rand() % 1024) / 1024.0f) * 2.0f - 1.0f) * 0.1f;
			Entity* entity;
			GameNode3D* node;
			((ItemDrop*)EntityTemplates::getInstance()->getTemplate("item"))->create(item, player->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition(), glm::vec3(vx,vy,vz), entity, node);
			Core::getRoot()->addChild(node);
			World::currentWorld->entityManager->registerEntity(entity);	
		}
	}
	else 
	{
		PlayerCraftByIndexPacket packet;
		packet.packetType = PlayerManagerPacketType::craft;
		packet.craftIndex = index;
		Core::client.send(World::currentWorld->playerManager->getNetworkId(), &packet, sizeof(packet));
	}
}
