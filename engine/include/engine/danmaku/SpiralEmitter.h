#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/BulletEmitter.h"

namespace Engine {

// Spawns one bullet (or one per arm) at a steadily rotating angle every
// spawnInterval seconds - each new bullet is offset from the last, so the
// stream traces a spiral outward as they all fly off at constant speed.
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
