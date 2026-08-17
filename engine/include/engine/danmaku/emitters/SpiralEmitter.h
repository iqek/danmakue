#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/emitters/BulletEmitter.h"

namespace Engine {

// Fires at a steadily rotating angle, tracing a spiral outward over time
class SpiralEmitter : public BulletEmitter {
private:
	float bulletSpeed;
	float spawnInterval;
	float angularVelocity;
	int arms;
	glm::vec4 color;
	float spawnTimer = 0.0f;
	float currentAngle = 0.0f;

public:
	SpiralEmitter(float bulletSpeed, float spawnInterval, float angularVelocity, int arms, glm::vec4 color);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition) override;
};

}
