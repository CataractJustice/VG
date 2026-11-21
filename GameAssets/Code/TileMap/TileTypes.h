#include <vector>
#include <string>
#include <unordered_map>
#include "libs/glm/glm.hpp"
struct TileType 
{
	bool isNode;
	bool isCube;
	unsigned short texture;
	std::string nodePath;
};

class TileTypes 
{
	private:
	TileTypes();
	void addType(const std::string& name, const TileType& typedescription);
	static TileTypes* instance;
	std::vector<TileType> types;
	std::unordered_map<std::string, unsigned short> nameIdMap;
	public:
	const TileType& getById(unsigned short id) const;
	static const TileTypes& getInstance();
	unsigned short getTypeIdByName(const std::string& name) const;
};