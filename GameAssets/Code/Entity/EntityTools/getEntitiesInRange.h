#include "glm/glm.hpp"
#include <map>
#include <ranges>
class Entity;
std::map<float, Entity*> getEntitiesInRange(std::initializer_list<unsigned int> types, glm::vec3 position, float range);