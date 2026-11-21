#include "GameNode/Component.h"
#include <filesystem>
using namespace TealEngine;
namespace TealEngine 
{
	class TextInput;
	class GameNodeGUI;
}
class MenuInit : public Component 
{
	static MenuInit* instance;
	GameNodeGUI* currentMenu = nullptr;
	std::vector<GameNodeGUI*> menuStack;
	std::map<std::string, TextInput*> textInputs;
	public:
	void onAttach() override;
	void openMenu(const std::filesystem::path& path);
	void popMenu();
	GameNodeGUI* newItem(const std::string& title);
	static MenuInit* getMenuInstance();
	const std::string& getInputValue(const std::string& inputName) const;
};