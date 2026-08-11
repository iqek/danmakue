#include "engine/danmaku/RandomScatterEmitter.h"
#include "engine/danmaku/BulletPool.h"

#include <cmath>
#include <glm/gtc/constants.hpp>

namespace Engine {

RandomScatterEmitter::RandomScatterEmitter(glm::vec2 origin, float minSpeed, float maxSpeed, float spawnInterval, glm::vec4 color):
	origin(origin), spawnInterval(spawnInterval), color(color),
	randomEngine(std::random_device{}()),
	angleDist(0.0f, glm::two_pi<float>()),
	speedDist(minSpeed, maxSpeed)
{
}

void RandomScatterEmitter::Update(float deltaTime, BulletPool& bulletPool, glm::vec2 targetPosition){
	timer -= deltaTime;
	if(timer > 0.0f){
		return;
	}
	timer = spawnInterval;

	float angle = angleDist(randomEngine);
	float speed = speedDist(randomEngine);
	glm::vec2 direction(std::cos(angle), std::sin(angle));
	bulletPool.Spawn(origin, direction * speed, 4.0f, color);
}

}
