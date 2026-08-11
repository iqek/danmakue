#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/BulletEmitter.h"

namespace Engine {

// spawns bulletCount bullets evenly spaced around a full circle, all at once every interval
class RadialBurstEmitter : public BulletEmitter {
private:
	glm::vec2 origin;
	int bulletCount;
	float bulletSpeed;
	float interval;
	glm::vec4 color;
	float timer = 0.0f;

public:
	RadialBurstEmitter(glm::vec2 origin, int bulletCount, float bulletSpeed, float interval, glm::vec4 color);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 targetPosition) override;
};

}
