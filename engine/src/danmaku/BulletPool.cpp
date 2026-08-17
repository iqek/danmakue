#include "engine/danmaku/BulletPool.h"
#include "engine/render/Renderer2D.h"

namespace Engine {

namespace {

constexpr float despawnMargin = 32.0f;

}

BulletPool::BulletPool(std::size_t capacity, glm::vec2 boundsMin, glm::vec2 boundsMax):
	boundsMin(boundsMin), boundsMax(boundsMax)
{
	bullets.resize(capacity);
}

void BulletPool::Spawn(glm::vec2 position, glm::vec2 velocity, float radius, glm::vec4 color, glm::vec2 acceleration, float homingStrength){
	for(auto& bullet : bullets){
		if(!bullet.active){
			bullet.position = position;
			bullet.velocity = velocity;
			bullet.acceleration = acceleration;
			bullet.radius = radius;
			bullet.color = color;
			bullet.homingStrength = homingStrength;
			bullet.active = true;
			return;
		}
	}
	// Pool full: bullet silently dropped, a hard cap is intentional here
}

void BulletPool::Update(float deltaTime, glm::vec2 targetPosition){
	glm::vec2 min = boundsMin - glm::vec2(despawnMargin);
	glm::vec2 max = boundsMax + glm::vec2(despawnMargin);

	for(auto& bullet : bullets){
		if(!bullet.active){
			continue;
		}

		if(bullet.homingStrength > 0.0f){
			glm::vec2 toTarget = targetPosition - bullet.position;
			float distanceToTarget = glm::length(toTarget);
			float speed = glm::length(bullet.velocity);
			if(distanceToTarget > 0.0001f && speed > 0.0001f){
				glm::vec2 desiredDirection = toTarget / distanceToTarget;
				glm::vec2 currentDirection = bullet.velocity / speed;
				float turnAmount = glm::clamp(bullet.homingStrength * deltaTime, 0.0f, 1.0f);
				glm::vec2 turnedDirection = glm::normalize(glm::mix(currentDirection, desiredDirection, turnAmount));
				bullet.velocity = turnedDirection * speed;
			}
		}

		bullet.velocity += bullet.acceleration * deltaTime;
		bullet.position += bullet.velocity * deltaTime;

		bool outOfBounds = bullet.position.x < min.x || bullet.position.x > max.x || bullet.position.y < min.y || bullet.position.y > max.y;
		if(outOfBounds){
			bullet.active = false;
		}
	}
}

void BulletPool::Draw(const Renderer2D& renderer) const{
	for(const auto& bullet : bullets){
		if(!bullet.active){
			continue;
		}

		renderer.DrawQuad(bullet.position, glm::vec2(bullet.radius * 2.0f), bullet.color);
	}
}

bool BulletPool::CheckCollision(glm::vec2 boxCenter, glm::vec2 boxSize) const{
	glm::vec2 boxMin = boxCenter - boxSize * 0.5f;
	glm::vec2 boxMax = boxCenter + boxSize * 0.5f;

	for(const auto& bullet : bullets){
		if(!bullet.active){
			continue;
		}

		glm::vec2 bulletMin = bullet.position - glm::vec2(bullet.radius);
		glm::vec2 bulletMax = bullet.position + glm::vec2(bullet.radius);

		bool overlap = boxMin.x < bulletMax.x && boxMax.x > bulletMin.x && boxMin.y < bulletMax.y && boxMax.y > bulletMin.y;
		if(overlap){
			return true;
		}
	}

	return false;
}

bool BulletPool::ConsumeCollisions(glm::vec2 boxCenter, glm::vec2 boxSize){
	glm::vec2 boxMin = boxCenter - boxSize * 0.5f;
	glm::vec2 boxMax = boxCenter + boxSize * 0.5f;

	bool hitAny = false;
	for(auto& bullet : bullets){
		if(!bullet.active){
			continue;
		}

		glm::vec2 bulletMin = bullet.position - glm::vec2(bullet.radius);
		glm::vec2 bulletMax = bullet.position + glm::vec2(bullet.radius);

		bool overlap = boxMin.x < bulletMax.x && boxMax.x > bulletMin.x && boxMin.y < bulletMax.y && boxMax.y > bulletMin.y;
		if(overlap){
			bullet.active = false;
			hitAny = true;
		}
	}

	return hitAny;
}

}
