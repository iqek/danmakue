#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Engine {

class BulletPool;

// Advances each enemy's movement, then fires its emitters from its current position
void UpdateEnemies(entt::registry& registry, BulletPool& bulletPool, float deltaTime, glm::vec2 targetPosition);

// A hit costs damagePerHit health and consumes the bullet; dead enemies award score
void UpdateEnemyHealth(entt::registry& registry, BulletPool& playerBulletPool, int damagePerHit, entt::entity player, int scorePerKill);

// Destroys enemies that have drifted well outside the stage bounds, no score awarded
void DespawnOffscreenEnemies(entt::registry& registry, glm::vec2 boundsMin, glm::vec2 boundsMax);

}
