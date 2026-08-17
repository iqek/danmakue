#pragma once

#include <entt/entt.hpp>

namespace Engine {

class BulletPool;

// Ticks invincibility down; once expired, a bullet hit costs a life and resets it
void UpdatePlayerDamage(entt::registry& registry, entt::entity player, const BulletPool& bulletPool, float deltaTime);

// Fires the player's weapons, if any, while firing is true
void UpdatePlayerWeapons(entt::registry& registry, entt::entity player, BulletPool& bulletPool, float deltaTime, bool firing);

}
