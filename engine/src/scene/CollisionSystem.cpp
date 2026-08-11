#include "engine/scene/CollisionSystem.h"
#include "engine/scene/Components.h"

#include <glm/glm.hpp>

namespace Engine {

bool CheckCollision(entt::registry& registry, entt::entity a, entt::entity b){
	const auto& transformA = registry.get<Transform>(a);
	const auto& colliderA = registry.get<Collider>(a);
	const auto& transformB = registry.get<Transform>(b);
	const auto& colliderB = registry.get<Collider>(b);

	glm::vec2 minA = transformA.position - colliderA.size * 0.5f;
	glm::vec2 maxA = transformA.position + colliderA.size * 0.5f;
	glm::vec2 minB = transformB.position - colliderB.size * 0.5f;
	glm::vec2 maxB = transformB.position + colliderB.size * 0.5f;

	return minA.x < maxB.x && maxA.x > minB.x && minA.y < maxB.y && maxA.y > minB.y;
}

std::vector<std::pair<entt::entity, entt::entity>> FindCollisions(entt::registry& registry){
	std::vector<std::pair<entt::entity, entt::entity>> collisions;

	auto view = registry.view<Transform, Collider>();
	for(auto it = view.begin(); it != view.end(); ++it){
		for(auto other = std::next(it); other != view.end(); ++other){
			if(CheckCollision(registry, *it, *other)){
				collisions.emplace_back(*it, *other);
			}
		}
	}

	return collisions;
}

}
