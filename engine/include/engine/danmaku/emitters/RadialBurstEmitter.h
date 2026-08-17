#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/emitters/BulletEmitter.h"

namespace Engine {

// Spawns bulletCount bullets in a ring at once, repeating every interval
class RadialBurstEmitter : public BulletEmitter {
private:
	int bulletCount;
	float bulletSpeed;
	float interval;
	glm::vec4 color;
	float timer = 0.0f;

public:
	RadialBurstEmitter(int bulletCount, float bulletSpeed, float interval, glm::vec4 color);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition) override;
};

}
