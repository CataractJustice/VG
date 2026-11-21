#include "GameNode/Component.h"
using namespace TealEngine;
class JoinServerButton : public Component 
{
	private:
	public:
	void onClientConnect() override;
	void onClientConnectFail() override;
	void onClientDisconnect() override;
	void setWorldName(const std::string& name);
	JoinServerButton();
	void onClick(unsigned short button) override;
};