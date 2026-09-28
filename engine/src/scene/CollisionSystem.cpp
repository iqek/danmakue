#include "engine/scene/CollisionSystem.h"

#include <glm/glm.hpp>

namespace Engine {

namespace {

bool BoxOverlapsBox(glm::vec2 centerA, glm::vec2 sizeA, glm::vec2 centerB, glm::vec2 sizeB){
	glm::vec2 minA = centerA - sizeA * 0.5f;
	glm::vec2 maxA = centerA + sizeA * 0.5f;
	glm::vec2 minB = centerB - sizeB * 0.5f;
	glm::vec2 maxB = centerB + sizeB * 0.5f;

	return minA.x < maxB.x && maxA.x > minB.x && minA.y < maxB.y && maxA.y > minB.y;
}

bool CircleOverlapsCircle(glm::vec2 centerA, float radiusA, glm::vec2 centerB, float radiusB){
	float reach = radiusA + radiusB;
	glm::vec2 offset = centerB - centerA;
	return glm::dot(offset, offset) < reach * reach;
}

// the closest point on the box to the circle's centre is what decides it
bool CircleOverlapsBox(glm::vec2 circleCenter, float radius, glm::vec2 boxCenter, glm::vec2 boxSize){
	glm::vec2 half = boxSize * 0.5f;
	glm::vec2 nearest = glm::clamp(circleCenter, boxCenter - half, boxCenter + half);
	glm::vec2 offset = circleCenter - nearest;
	return glm::dot(offset, offset) < radius * radius;
}

}

bool Overlaps(glm::vec2 centerA, const Collider& a, glm::vec2 centerB, const Collider& b){
	bool circleA = a.shape == Collider::Shape::Circle;
	bool circleB = b.shape == Collider::Shape::Circle;

	if(circleA && circleB){
		return CircleOverlapsCircle(centerA, a.radius, centerB, b.radius);
	}
	if(circleA){
		return CircleOverlapsBox(centerA, a.radius, centerB, b.size);
	}
	if(circleB){
		return CircleOverlapsBox(centerB, b.radius, centerA, a.size);
	}
	return BoxOverlapsBox(centerA, a.size, centerB, b.size);
}

bool CheckCollision(entt::registry& registry, entt::entity a, entt::entity b){
	return Overlaps(registry.get<Transform>(a).position, registry.get<Collider>(a), registry.get<Transform>(b).position, registry.get<Collider>(b));
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
