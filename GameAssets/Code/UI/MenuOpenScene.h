#include "GameNode/Component.h"
using namespace TealEngine;
class MenuOpenScene : public Component 
{
	public:
	std::string scenePath;
	MenuOpenScene();
	void onClick(unsigned short button) override;
};