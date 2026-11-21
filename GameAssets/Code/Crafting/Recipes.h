#pragma once
#include <map>
#include <vector>
#include <string>

class Recipe 
{
	private:
	std::map<std::string, unsigned int> inputItemCountByItemId;
	std::map<std::string, unsigned int> outputItemCountByItemId;
	public:
	template<typename InputRange, typename OutputRange>
	Recipe(const InputRange& inputs, const OutputRange& outputs)
	{
		for (const auto& item : inputs)
			inputItemCountByItemId[item.first] = item.second;
		for (const auto& item : outputs)
			outputItemCountByItemId[item.first] = item.second;
	}
	Recipe(std::initializer_list<std::pair<std::string, unsigned int>> inputs, std::initializer_list<std::pair<std::string, unsigned int>> outputs)
	{
		for (const auto& item : inputs)
			inputItemCountByItemId[item.first] = item.second;
		for (const auto& item : outputs)
			outputItemCountByItemId[item.first] = item.second;
	}
	Recipe(const Recipe&) = default;
	Recipe& operator=(const Recipe&) = default;
	Recipe(Recipe&&) = default;
	Recipe& operator=(Recipe&&) = default;
	bool checkRequirements(const std::map<std::string, unsigned int>& availableMaterials) const;
	inline const std::map<std::string, unsigned int>& getRequiredItems() const { return inputItemCountByItemId; }
	inline const std::map<std::string, unsigned int>& getOutputItems() const { return outputItemCountByItemId; }
};

class Recipes 
{
	private:
	std::vector<Recipe> listOfRecipes;
	public:
	inline void addRecipe(const Recipe& recipe) 
	{
		this->listOfRecipes.push_back(recipe);
	}
	inline const std::vector<Recipe>& getRecipes() const 
	{
		return listOfRecipes;
	}
};