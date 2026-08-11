#include "engine/danmaku/AimedShotEmitter.h"
#include "engine/danmaku/BulletPool.h"

namespace Engine {

AimedShotEmitter::AimedShotEmitter(glm::vec2 origin, float bulletSpeed, float interval, glm::vec4 color):
	origin(origin), bulletSpeed(bulletSpeed), interval(interval), color(color)
{
}

void AimedShotEmitter::Update(float deltaTime, BulletPool& bulletPool, glm::vec2 targetPosition){
	timer -= deltaTime;
	if(timer > 0.0f){
		return;
	}
	timer = interval;

	glm::vec2 toTarget = targetPosition - origin;
	if(glm::length(toTarget) < 0.0001f){
		return;
	}

	glm::vec2 direction = glm::normalize(toTarget);
	bulletPool.Spawn(origin, direction * bulletSpeed, 5.0f, color);
}

}
