#include "GameNode/Component.h"
using namespace TealEngine;
class GameStartButton : public Component 
{
	private:
	std::string worldName;
	public:
	void setWorldName(const std::string& name);
	GameStartButton();
	void onClick(unsigned short button) override;
};