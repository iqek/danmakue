#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/BulletEmitter.h"

namespace Engine {

// fires one bullet straight at targetPosition every interval
class AimedShotEmitter : public BulletEmitter {
private:
	glm::vec2 origin;
	float bulletSpeed;
	float interval;
	glm::vec4 color;
	float timer = 0.0f;

public:
	AimedShotEmitter(glm::vec2 origin, float bulletSpeed, float interval, glm::vec4 color);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 targetPosition) override;
};

}
