#include "engine/danmaku/emitters/RadialBurstEmitter.h"
#include "engine/danmaku/BulletPool.h"

#include <cmath>
#include <glm/gtc/constants.hpp>

namespace Engine {

RadialBurstEmitter::RadialBurstEmitter(int bulletCount, float bulletSpeed, float interval, glm::vec4 color):
	bulletCount(bulletCount), bulletSpeed(bulletSpeed), interval(interval), color(color)
{
}

void RadialBurstEmitter::Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition){
	timer -= deltaTime;
	if(timer > 0.0f){
		return;
	}
	timer = interval;

	float angleStep = glm::two_pi<float>() / static_cast<float>(bulletCount);
	for(int i = 0; i < bulletCount; i++){
		float angle = angleStep * static_cast<float>(i);
		glm::vec2 direction(std::cos(angle), std::sin(angle));
		bulletPool.Spawn(originPosition, direction * bulletSpeed, 5.0f, color);
	}
}

}
