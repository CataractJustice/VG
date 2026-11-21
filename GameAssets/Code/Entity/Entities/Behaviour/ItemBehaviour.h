#include "GameNode/Component.h"
#include "Inventory/InventoryItem.h"
#include "EntityNetworkListener.h"
using namespace TealEngine;
class ItemBehaviour : public EntityNetworkListener
{
	private:
	InventoryItem item;
	unsigned char mergeAttemptTickOffset;
	glm::vec3 velocity;
	glm::vec3 lastSyncPosition;
	public:
	ItemBehaviour(const InventoryItem& item);
	unsigned int ticksExisting;
	void update() override;
	inline InventoryItem& getItem() { return item; }
	inline const InventoryItem& getItem() const { return item; }
	inline glm::vec3 getVelocity() const { return velocity; };
	inline void setVelocity(glm::vec3 velocity) { this->velocity = velocity; };
	void onClientEntityPacket(const void* data, unsigned int size) override;
};