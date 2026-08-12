#include "engine/danmaku/AimedShotEmitter.h"
#include "engine/danmaku/BulletPool.h"

namespace Engine {

AimedShotEmitter::AimedShotEmitter(float bulletSpeed, float interval, glm::vec4 color):
	bulletSpeed(bulletSpeed), interval(interval), color(color)
{
}

void AimedShotEmitter::Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition){
	timer -= deltaTime;
	if(timer > 0.0f){
		return;
	}
	timer = interval;

	glm::vec2 toTarget = targetPosition - originPosition;
	if(glm::length(toTarget) < 0.0001f){
		return;
	}

	glm::vec2 direction = glm::normalize(toTarget);
	bulletPool.Spawn(originPosition, direction * bulletSpeed, 5.0f, color);
}

}
