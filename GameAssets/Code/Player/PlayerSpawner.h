#include "GameNode/Component.h"
#include "Player/PlayerBuilder.h"
using namespace TealEngine;

class PlayerSpawner : public Component 
{
	private:
	bool spawned;
	bool loadPlayer = false;
	bool tryGroundPlayer = true;
	GameNode3D* player;
	PlayerBuilder* pb;	
	std::string playerUUID;
	public:
	PlayerSpawner(GameNode3D* node = nullptr, PlayerBuilder* playerBuilder = nullptr, const std::string& uuid = "");
	void onAttach() override;
	void update() override;
};