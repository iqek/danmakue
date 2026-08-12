#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Engine {

class BulletPool;

// Updates every entity with a Transform + Enemy: advances its movement (if
// any), then updates each of its emitters using the enemy's current
// position as the spawn origin.
void UpdateEnemies(entt::registry& registry, BulletPool& bulletPool, float deltaTime, glm::vec2 targetPosition);

// Checks every entity with a Transform + Collider + Enemy against
// playerBulletPool; a hit costs damagePerHit health and consumes the
// bullet(s) that caused it. Enemies at 0 health are destroyed and award
// scorePerKill to player's score.
void UpdateEnemyHealth(entt::registry& registry, BulletPool& playerBulletPool, int damagePerHit, entt::entity player, int scorePerKill);

}
