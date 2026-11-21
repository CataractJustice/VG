#include "MenuInit.h"
#include <fstream>
#include "FileSystem/loadStrFromFile.h"
#include "GameNode/GameNodeGUI.h"
#include "GameNode/ComponentFactory.h"
#include "Graphics/GUI/Background.h"
#include "Graphics/GUI/MonospaceBitmapText.h"
#include "Core.h"
#include "Graphics/GUI/TextInput.h"
#include "Graphics/GUI/ScrollOnHover.h"
#include <format>
MenuInit* MenuInit::instance = nullptr;
const std::filesystem::path menuDir = "./GameAssets/Menu/";
const GUIUnit buttonHeight = GUIUnit::pixels(32.0f);
const GUIUnit buttonPadding = GUIUnit::pixels(5.0f);
const GUIUnit textPadding = GUIUnit::pixels(8.0f);
const glm::vec4 buttonColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
const glm::vec4 textColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
const glm::vec4 inputColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);

class MenuOpenOnClick : public Component 
{
	public:
	MenuInit* menu;
	std::filesystem::path path;
	void onClick(unsigned short button) override 
	{
		menu->openMenu(path);
	}
};

class MenuBackOnClick : public Component 
{
	public:
	MenuInit* menu;
	void onClick(unsigned short button) override 
	{
		menu->popMenu();
	}
};

class MenuQuitOnClick : public Component 
{
	public:

	void onClick(unsigned short button) override 
	{
		exit(0);
	}
};

void MenuInit::onAttach() 
{
	instance = this;
	openMenu(menuDir / "mainmenu.ini");
}

void MenuInit::openMenu(const std::filesystem::path& path) 
{
	std::string menuText = loadStrFromFile(path);
	std::stringstream menuTextStream(menuText);
	if(currentMenu) 
	{
		this->getParent()->removeChild(currentMenu);
		menuStack.push_back(currentMenu);
	}
	GameNodeGUI* menuRoot = new GameNodeGUI();
	menuRoot->rename(std::format("Menu[{}]", path.filename().string()));
	const float menuWidth = 1.0f;
	menuRoot->getTransform().setWidth(GUIUnit::parentWidths(menuWidth));
	menuRoot->getTransform().setOffsetX(GUIUnit::parentWidths(0.0f));

	this->getParent()->addChild(menuRoot);
	currentMenu = menuRoot;
	std::string title;
	while(std::getline(menuTextStream, title, '\n')) 
	{
		GameNodeGUI* itemElement = (GameNodeGUI*)newItem(title);
		std::string line;
		while(std::getline(menuTextStream, line, '\n')) 
		{
			std::string token;
			std::stringstream tokenStream(line);
			std::getline(tokenStream, token, ' ');
			if(token == "menu") 
			{
				std::getline(tokenStream, token, ' ');
				MenuOpenOnClick* menuOpenComponent = new MenuOpenOnClick();
				menuOpenComponent->menu = this;

				menuOpenComponent->path = menuDir / token;
				itemElement->attachComponent(menuOpenComponent);
			}
			if(token == "input") 
			{
				std::string inputName;
				std::getline(tokenStream, inputName, ' ');
				GameNodeGUI* inputNode = new GameNodeGUI();
				itemElement->addChild(inputNode);
				inputNode->getTransform().setHeight(GUIUnit::parentHeights(1.0f) - textPadding);
				inputNode->getTransform().setOffsetY(textPadding / 2.0f);
				inputNode->getTransform().setWidth(GUIUnit::parentWidths(0.5f) - textPadding);
				inputNode->getTransform().setOffsetX(GUIUnit::parentWidths(0.5f) + textPadding / 2.0f);
				TextInput* input = new TextInput();
				Background* inputBg = new Background();
				inputBg->setColor(inputColor);
				MonospaceBitmapText* textDisplay = new MonospaceBitmapText();
				textDisplay->setColor(textColor);
				textDisplay->setTexture(&Core::textureManager.get("11x11font.png"));
				textDisplay->setText("");
				textDisplay->setLeft(textPadding / 2.0f);
				textDisplay->setTop(textPadding / 2.0f);
				textDisplay->setSize(GUIUnit::parentHeights(1.0f) - textPadding - textPadding);
				input->textDisplay = textDisplay;
				inputNode->attachComponents({input, inputBg, textDisplay});
				textInputs[inputName] = input;
			}
			if(token == "comp") 
			{
				std::string componentName;
				std::getline(tokenStream, componentName, ' ');
				Component* component = ComponentFactory::instanceComponent(componentName);
				itemElement->attachComponent(component);
			}
			if(line[0] == '=') 
			{
				break;
			}
		}
	}

	if(this->menuStack.size() == 0) 
	{
		GameNodeGUI* backButton = (GameNodeGUI*)newItem("Quit");
		backButton->rename("Quit");
		backButton->attachComponent(new MenuQuitOnClick());
	}
	else 
	{
		GameNodeGUI* backButton = (GameNodeGUI*)newItem("Back");
		backButton->rename("Back");
		MenuBackOnClick* mboc = new MenuBackOnClick();
		mboc->menu = this;
		backButton->attachComponent(mboc);
	}
}

void MenuInit::popMenu() 
{
	currentMenu->destroy();
	this->getParent()->addChild(menuStack.back());
	currentMenu = menuStack.back();
	menuStack.pop_back();
}

const std::string& MenuInit::getInputValue(const std::string& inputName) const
{
	return textInputs.find(inputName)->second->getInputText();
}

MenuInit* MenuInit::getMenuInstance() 
{
	return instance;
}

GameNodeGUI* MenuInit::newItem(const std::string& title) 
{
	GameNodeGUI* itemElement = new GameNodeGUI();
	itemElement->rename(title);
	itemElement->getTransform().setWidth(GUIUnit::parentWidths(1.0f));
	itemElement->getTransform().setHeight(buttonHeight);
	itemElement->getTransform().setOffsetX(GUIUnit::pixels(0));
	itemElement->getTransform().setOffsetY((buttonHeight + buttonPadding) * currentMenu->getChilds().size());

	currentMenu->getTransform().setHeight(itemElement->getTransform().getOffsetY() + itemElement->getTransform().getHeight());

	Background* bg = new Background();
	bg->setColor(buttonColor);
	MonospaceBitmapText* text = new MonospaceBitmapText();
	text->setColor(textColor);
	text->setTexture(&Core::textureManager.get("11x11font.png"));
	text->setSize(buttonHeight - textPadding - textPadding);
	text->setTop(textPadding);
	text->setLeft(textPadding);
	text->setText(title);
	itemElement->attachComponents({bg, text});

	currentMenu->addChild(itemElement);
	return itemElement;
}
EXPORT_COMPONENT(MenuInit);