#include "engine/danmaku/EnemySystem.h"
#include "engine/danmaku/BulletPool.h"
#include "engine/danmaku/Enemy.h"
#include "engine/danmaku/Player.h"
#include "engine/scene/Components.h"

#include <vector>

namespace Engine {

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

	// Entities can't be safely destroyed mid-iteration of a view over their
	// own component types, so collect the dead ones and destroy them after.
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

}
