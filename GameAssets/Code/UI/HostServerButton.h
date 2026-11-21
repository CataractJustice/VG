#include "GameNode/Component.h"
using namespace TealEngine;
class HostServerButton : public Component 
{
	private:
	public:
	//called on a host every time a new client is connected
	void onHostPeerConnect(PeerInfo& peerInfo) override;
	//called on a host every time a client disconnects
	void onHostPeerDisconnect(PeerInfo& peerInfo) override;
	//called on a host every time a packet is received from a client
	void onHostPeerPacket(PeerInfo& peerInfo, const void* data, int size) override;
	void setWorldName(const std::string& name);
	void onClick(unsigned short button) override;
};