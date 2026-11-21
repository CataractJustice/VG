
#include "Component.h"
using namespace TealEngine;
class World;
class WorldNetworkManager : public Component
{
	private:
	World* world;
	void handleChunkDataClient(void* data, int size);

	void handleChunkRequestHost(PeerInfo& peer, void* data, int size);
	
	public:
	WorldNetworkManager(World* world);
	void onClientPacket(const void* data, int size) override;
	void onHostPeerPacket(PeerInfo& peer, const void* data, int size) override;
	void onHostPeerConnect(PeerInfo& peer) override;
};