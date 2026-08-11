#pragma once

#include <glm/glm.hpp>
#include <random>

#include "engine/danmaku/BulletEmitter.h"

namespace Engine {

// fires one bullet at a random angle and random speed every interval
// this one is deliberately chaotic.
class RandomScatterEmitter : public BulletEmitter {
private:
	glm::vec2 origin;
	float spawnInterval;
	glm::vec4 color;
	float timer = 0.0f;
	std::mt19937 randomEngine;
	std::uniform_real_distribution<float> angleDist;
	std::uniform_real_distribution<float> speedDist;

public:
	RandomScatterEmitter(glm::vec2 origin, float minSpeed, float maxSpeed, float spawnInterval, glm::vec4 color);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 targetPosition) override;
};

}
