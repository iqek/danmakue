#pragma once

#include <entt/entt.hpp>

namespace Engine {

class BulletPool;

// Ticks the player's invincibility timer down. Once it's expired, checks the
// player's Collider against the bullet pool directly (bullets aren't EnTT
// entities) - on a hit, costs one life and starts a new invincibility window.
void UpdatePlayerDamage(entt::registry& registry, entt::entity player, const BulletPool& bulletPool, float deltaTime);

// Fires the player's weapons (if any) into bulletPool while firing is true.
// A player with no weapons attached simply does nothing here - not every
// game needs the player to shoot at all.
void UpdatePlayerWeapons(entt::registry& registry, entt::entity player, BulletPool& bulletPool, float deltaTime, bool firing);

}
