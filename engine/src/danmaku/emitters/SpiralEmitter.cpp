#include "engine/danmaku/emitters/SpiralEmitter.h"
#include "engine/danmaku/BulletPool.h"

#include <cmath>
#include <glm/gtc/constants.hpp>

namespace Engine {

SpiralEmitter::SpiralEmitter(float bulletSpeed, float spawnInterval, float angularVelocity, int arms, glm::vec4 color):
	bulletSpeed(bulletSpeed), spawnInterval(spawnInterval), angularVelocity(angularVelocity), arms(arms), color(color)
{
}

void SpiralEmitter::Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition){
	currentAngle += angularVelocity * deltaTime;

	spawnTimer -= deltaTime;
	if(spawnTimer > 0.0f){
		return;
	}
	spawnTimer = spawnInterval;

	float armStep = glm::two_pi<float>() / static_cast<float>(arms);
	for(int i = 0; i < arms; i++){
		float angle = currentAngle + armStep * static_cast<float>(i);
		glm::vec2 direction(std::cos(angle), std::sin(angle));
		bulletPool.Spawn(originPosition, direction * bulletSpeed, 5.0f, color);
	}
}

}
