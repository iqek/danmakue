#pragma once

#include <glm/glm.hpp>
#include <random>

#include "engine/danmaku/emitters/BulletEmitter.h"

namespace Engine {

// Fires at a random angle and speed every spawnInterval; deliberately chaotic.
// outwardAcceleration > 0 makes each bullet keep speeding up as it flies.
class RandomScatterEmitter : public BulletEmitter {
private:
	float spawnInterval;
	glm::vec4 color;
	float outwardAcceleration;
	float timer = 0.0f;
	std::mt19937 randomEngine;
	std::uniform_real_distribution<float> angleDist;
	std::uniform_real_distribution<float> speedDist;

public:
	RandomScatterEmitter(float minSpeed, float maxSpeed, float spawnInterval, glm::vec4 color, float outwardAcceleration = 0.0f);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition) override;
};

}
