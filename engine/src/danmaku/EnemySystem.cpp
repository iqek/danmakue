#include "engine/danmaku/EnemySystem.h"
#include "engine/danmaku/BulletPool.h"
#include "engine/danmaku/Enemy.h"
#include "engine/danmaku/Player.h"
#include "engine/scene/Components.h"

#include <vector>

namespace Engine {

namespace {

constexpr float despawnMargin = 100.0f;

}

void UpdateEnemies(entt::registry& registry, BulletPool& bulletPool, float deltaTime, glm::vec2 targetPosition){
	auto view = registry.view<Transform, Enemy>();
	for(auto entity : view){
		auto& transform = view.get<Transform>(entity);
		auto& enemy = view.get<Enemy>(entity);

		if(enemy.movement){
			enemy.movement->Update(deltaTime, transform);
		}

		for(auto& emitter : enemy.emitters){
			emitter->Update(deltaTime, bulletPool, transform.position, targetPosition);
		}
	}
}

void UpdateEnemyHealth(entt::registry& registry, BulletPool& playerBulletPool, int damagePerHit, entt::entity player, int scorePerKill){
	auto view = registry.view<Transform, Collider, Enemy>();

	// Can't destroy entities mid-view-iteration, so collect them and destroy after
	std::vector<entt::entity> toDestroy;
	for(auto entity : view){
		auto& transform = view.get<Transform>(entity);
		auto& collider = view.get<Collider>(entity);
		auto& enemy = view.get<Enemy>(entity);

		if(playerBulletPool.ConsumeCollisions(transform.position, collider.size)){
			enemy.health -= damagePerHit;
			if(enemy.health <= 0){
				toDestroy.push_back(entity);
			}
		}
	}

	if(!toDestroy.empty()){
		registry.get<Player>(player).score += scorePerKill * static_cast<int>(toDestroy.size());
	}

	for(auto entity : toDestroy){
		registry.destroy(entity);
	}
}

void DespawnOffscreenEnemies(entt::registry& registry, glm::vec2 boundsMin, glm::vec2 boundsMax){
	glm::vec2 min = boundsMin - glm::vec2(despawnMargin);
	glm::vec2 max = boundsMax + glm::vec2(despawnMargin);

	auto view = registry.view<Transform, Enemy>();

	std::vector<entt::entity> toDestroy;
	for(auto entity : view){
		auto& transform = view.get<Transform>(entity);
		bool outOfBounds = transform.position.x < min.x || transform.position.x > max.x || transform.position.y < min.y || transform.position.y > max.y;
		if(outOfBounds){
			toDestroy.push_back(entity);
		}
	}

	for(auto entity : toDestroy){
		registry.destroy(entity);
	}
}

}
