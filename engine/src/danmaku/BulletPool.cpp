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

void BulletPool::Spawn(glm::vec2 position, glm::vec2 velocity, float radius, glm::vec4 color){
	for(auto& bullet : bullets){
		if(!bullet.active){
			bullet.position = position;
			bullet.velocity = velocity;
			bullet.radius = radius;
			bullet.color = color;
			bullet.active = true;
			return;
		}
	}
	// Pool is full - bullet is silently dropped.
	// (a hard cap on simultaneous bullets is normal, not just a missing feature.)
}

void BulletPool::Update(float deltaTime){
	glm::vec2 min = boundsMin - glm::vec2(despawnMargin);
	glm::vec2 max = boundsMax + glm::vec2(despawnMargin);

	for(auto& bullet : bullets){
		if(!bullet.active){
			continue;
		}

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
