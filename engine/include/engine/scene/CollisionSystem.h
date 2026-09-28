#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <utility>
#include <vector>

#include "engine/scene/Components.h"

namespace Engine {

// Overlap between two hitboxes placed at these positions, in any mix of shapes
bool Overlaps(glm::vec2 centerA, const Collider& a, glm::vec2 centerB, const Collider& b);

// The same test between two entities' Transform + Collider
bool CheckCollision(entt::registry& registry, entt::entity a, entt::entity b);

// Every overlapping pair; O(n^2), needs spatial partitioning at scale
std::vector<std::pair<entt::entity, entt::entity>> FindCollisions(entt::registry& registry);

}
