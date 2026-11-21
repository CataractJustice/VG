#include "glm/glm.hpp"
#include <vector>
#include <ranges>
class Entity;
Entity* getClosestEntityInRange(std::initializer_list<unsigned int> types, glm::vec3 position, float range);