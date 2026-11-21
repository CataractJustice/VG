#include "WorldList.h"
#include "FileSystem/getDirectoryList.h"
#include "Menu.h"
#include "World.h"
#include "GameNode/GameNodeGUI.h"
#include "Graphics/GUI/Background.h"
#include "Graphics/GUI/MonospaceBitmapText.h"
#include "GameStartButton.h"
#include "GameNode/ComponentFactory.h"
#include "Core.h"
void WorldList::onAttach() 
{
	std::vector<std::string> dirs = getDirectoryList("./data/worlds/");
	for(int i = 0; i < dirs.size(); i++) 
	{
		const std::string& dir = dirs[i];
		const std::string worldDirName = dir.substr(dir.find_last_of('/') + 1);
		GameNodeGUI* node = new GameNodeGUI();
		Background* bg = new Background();
		MonospaceBitmapText* textDisplay = new MonospaceBitmapText();
		
		GUITransform& t = node->getTransform();
		GUIUnit padding = GUIUnit::pixels(8);
		GUIUnit textSize = GUIUnit::pixels(22);
		GUIUnit height = padding + padding + textSize;
		GameStartButton* gsb = new GameStartButton();
		gsb->setWorldName(worldDirName);
		bg->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		bg->setPressColor(glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
		
		textDisplay->setText(dir);
		textDisplay->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		textDisplay->setLeft(padding);
		textDisplay->setTop(padding);
		textDisplay->setSize(textSize);
		textDisplay->setTexture(&Core::textureManager.get("11x11font.png"));
		
		t.setHeight(height);
		t.setWidth(GUIUnit::parentWidths(1.0f));
		t.setOffsetY((height + padding) * i);
		node->attachComponents({bg, textDisplay, gsb});
		this->getParent()->addChild(node);
	}
}

EXPORT_COMPONENT(WorldList);