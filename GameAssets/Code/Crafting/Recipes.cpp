#include "Recipes.h"

bool Recipe::checkRequirements(const std::map<std::string, unsigned int>& available) const 
{
	for(auto&[id, count] : inputItemCountByItemId) 
	{
		auto it = available.find(id);
		if(it == available.cend()) return false;
		if(it->second < count) return false;
	}
	return true;
}