#pragma once

#include <entt/entt.hpp>
#include <utility>
#include <vector>

namespace Engine {

// AABB overlap test between two entities' Transform + Collider
bool CheckCollision(entt::registry& registry, entt::entity a, entt::entity b);

// every pair of entities that currently overlap.
// O(n^2) over all colliders
// will need spatial partitioning once bullets increase by number
std::vector<std::pair<entt::entity, entt::entity>> FindCollisions(entt::registry& registry);

}
