#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/BulletEmitter.h"

namespace Engine {

// Fires one bullet straight at targetPosition every interval seconds.
class AimedShotEmitter : public BulletEmitter {
private:
	float bulletSpeed;
	float interval;
	glm::vec4 color;
	float timer = 0.0f;

public:
	AimedShotEmitter(float bulletSpeed, float interval, glm::vec4 color);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition) override;
};

}
