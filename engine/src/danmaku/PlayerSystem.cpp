#include "engine/danmaku/PlayerSystem.h"
#include "engine/danmaku/BulletPool.h"
#include "engine/danmaku/Player.h"
#include "engine/scene/Components.h"
#include "engine/core/Log.h"

namespace Engine {

namespace {

constexpr float invincibilityDuration = 1.5f;

}

void UpdatePlayerDamage(entt::registry& registry, entt::entity player, const BulletPool& bulletPool, float deltaTime){
	auto& playerData = registry.get<Player>(player);

	if(playerData.invincibleTimer > 0.0f){
		playerData.invincibleTimer -= deltaTime;
		return;
	}

	auto& transform = registry.get<Transform>(player);
	auto& collider = registry.get<Collider>(player);

	if(bulletPool.CheckCollision(transform.position, collider.size)){
		playerData.lives--;
		playerData.invincibleTimer = invincibilityDuration;
		ENGINE_CORE_INFO("Player hit! Lives remaining: {}", playerData.lives);
	}
}

void UpdatePlayerWeapons(entt::registry& registry, entt::entity player, BulletPool& bulletPool, float deltaTime, bool firing){
	if(!firing){
		return;
	}

	auto& transform = registry.get<Transform>(player);
	auto& playerData = registry.get<Player>(player);

	for(auto& weapon : playerData.weapons){
		weapon->Update(deltaTime, bulletPool, transform.position, transform.position);
	}
}

}
