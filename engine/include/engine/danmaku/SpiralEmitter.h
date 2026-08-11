#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/BulletEmitter.h"

namespace Engine {

// spawns one bullet at a steadily rotating angle every interval
// each new bullet is offset from the last, so the stream traces a spiral outward
class SpiralEmitter : public BulletEmitter {
private:
	glm::vec2 origin;
	float bulletSpeed;
	float spawnInterval;
	float angularVelocity;
	int arms;
	glm::vec4 color;
	float spawnTimer = 0.0f;
	float currentAngle = 0.0f;

public:
	SpiralEmitter(glm::vec2 origin, float bulletSpeed, float spawnInterval, float angularVelocity, int arms, glm::vec4 color);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 targetPosition) override;
};

}
