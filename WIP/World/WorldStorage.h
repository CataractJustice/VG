#pragma once
#include "libs/glm/glm.hpp"
#include <filesystem>
class WorldStorage 
{
	private:

	public:
	WorldStorage(std::filesystem::path directory);
	std::filesystem::path getStorageDirectory();
	std::filesystem::path getWorldInfoPath();
	std::filesystem::path getChunkPath(glm::ivec3 index);
};