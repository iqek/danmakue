#include "engine/danmaku/emitters/StraightShotEmitter.h"
#include "engine/danmaku/BulletPool.h"

namespace Engine {

StraightShotEmitter::StraightShotEmitter(glm::vec2 direction, float bulletSpeed, float interval, glm::vec4 color):
	direction(glm::normalize(direction)), bulletSpeed(bulletSpeed), interval(interval), color(color)
{
}

void StraightShotEmitter::Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition){
	timer -= deltaTime;
	if(timer > 0.0f){
		return;
	}
	timer = interval;

	bulletPool.Spawn(originPosition, direction * bulletSpeed, 5.0f, color);
}

}
