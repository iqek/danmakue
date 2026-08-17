#pragma once

#include <entt/entt.hpp>
#include <utility>
#include <vector>

namespace Engine {

// AABB overlap test between two entities' Transform + Collider
bool CheckCollision(entt::registry& registry, entt::entity a, entt::entity b);

// Every overlapping pair; O(n^2), needs spatial partitioning at scale
std::vector<std::pair<entt::entity, entt::entity>> FindCollisions(entt::registry& registry);

}
