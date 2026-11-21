#include "GameNode/Component.h"
using namespace TealEngine;
class LoadGameMenuInit : public Component 
{
	private:
	std::string worldName;
	public:
	LoadGameMenuInit();
	void onAttach() override;
};