#include "LoadGameMenuInit.h"
#include "../Menu/MenuInit.h"
#include <filesystem>
#include "GameNode/GameNodeGUI.h"
#include "GameStartButton.h"
#include "GameNode/ComponentFactory.h"
LoadGameMenuInit::LoadGameMenuInit() 
{

}

void LoadGameMenuInit::onAttach() 
{
	for(const auto& entry : std::filesystem::directory_iterator("./data/worlds/local")) 
	{
		if(!entry.is_directory()) continue;
		const std::string worldName = entry.path().filename().string();
		GameNodeGUI* gui = MenuInit::getMenuInstance()->newItem(worldName);
		GameStartButton* startBtn = new GameStartButton();
		startBtn->setWorldName(worldName);
		gui->attachComponent(startBtn);
	}
}

EXPORT_COMPONENT(LoadGameMenuInit);