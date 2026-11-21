#include "TileTypes.h"
#include <stdexcept>

// Initialize static instance pointer
TileTypes* TileTypes::instance = nullptr;

// -------------------- PRIVATE --------------------

TileTypes::TileTypes()
{
	addType("air", {false, false, 0, ""});
	addType("tiles", {false, true, 1, ""});
}

void TileTypes::addType(const std::string& name, const TileType& typedescription)
{
	// Store index before push_back
	unsigned short id = static_cast<unsigned short>(types.size());

	types.push_back(typedescription);
	nameIdMap[name] = id;
}

// -------------------- PUBLIC --------------------

const TileTypes& TileTypes::getInstance()
{
	if (!instance)
		instance = new TileTypes();
	return *instance;
}

const TileType& TileTypes::getById(unsigned short id) const
{
	if (id >= types.size())
		throw std::out_of_range("TileTypes::getById - invalid type id");

	return types[id];
}

unsigned short TileTypes::getTypeIdByName(const std::string& name) const
{
	auto it = nameIdMap.find(name);
	if (it == nameIdMap.end())
		throw std::out_of_range("TileTypes::getTypeIdByName - unknown tile type: " + name);

	return it->second;
}
