#include "engine/danmaku/PlayerSystem.h"
#include "engine/danmaku/BulletPool.h"
#include "engine/danmaku/Player.h"
#include "engine/render/Renderer2D.h"
#include "engine/scene/Components.h"
#include "engine/core/Log.h"

#include <cmath>

namespace Engine {

namespace {

// the bar drawn under the player while it is still intangible
constexpr float invincibilityBarHeight = 5.0f;
constexpr float invincibilityBarGap = 7.0f;

}

void UpdatePlayerDamage(entt::registry& registry, entt::entity player, const BulletPool& bulletPool, float deltaTime){
	auto& playerData = registry.get<Player>(player);

	if(playerData.invincibleTimer > 0.0f){
		playerData.invincibleTimer -= deltaTime;
		return;
	}

	auto& transform = registry.get<Transform>(player);
	auto& collider = registry.get<Collider>(player);

	if(bulletPool.CheckCollision(transform.position, collider)){
		playerData.lives--;
		playerData.invincibleTimer = playerData.invincibleDuration;
		ENGINE_CORE_INFO("Player hit! Lives remaining: {}", playerData.lives);
	}
}

void ShowPlayerInvincibility(entt::registry& registry, entt::entity player, const Renderer2D& renderer){
	auto& playerData = registry.get<Player>(player);
	auto& sprite = registry.get<Sprite>(player);

	if(playerData.invincibleTimer <= 0.0f || playerData.invincibleDuration <= 0.0f){
		sprite.color.a = 1.0f;
		return;
	}

	// a steady fade with a slow pulse reads as intangible; a fast strobe read as being hit
	sprite.color.a = 0.4f + 0.15f * std::sin(playerData.invincibleTimer * 14.0f);

	const auto& transform = registry.get<Transform>(player);
	float remaining = playerData.invincibleTimer / playerData.invincibleDuration;
	glm::vec2 center(transform.position.x, transform.position.y + transform.size.y * 0.5f + invincibilityBarGap);

	renderer.DrawQuad(center, glm::vec2(transform.size.x * remaining, invincibilityBarHeight), glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));
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
