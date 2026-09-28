#pragma once

#include <entt/entt.hpp>

namespace Engine {

class BulletPool;
class Renderer2D;

// Ticks invincibility down; once expired, a bullet hit costs a life and resets it
void UpdatePlayerDamage(entt::registry& registry, entt::entity player, const BulletPool& bulletPool, float deltaTime);

// Fades the player while it is intangible and shows how much of that is left.
// Call it once per frame after the screen is cleared and before the sprites go down.
void ShowPlayerInvincibility(entt::registry& registry, entt::entity player, const Renderer2D& renderer);

// Fires the player's weapons, if any, while firing is true
void UpdatePlayerWeapons(entt::registry& registry, entt::entity player, BulletPool& bulletPool, float deltaTime, bool firing);

}
