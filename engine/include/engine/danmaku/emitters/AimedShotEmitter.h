#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/emitters/BulletEmitter.h"

namespace Engine {

// Fires at targetPosition every interval; homingStrength > 0 lets it curve to follow
class AimedShotEmitter : public BulletEmitter {
private:
	float bulletSpeed;
	float interval;
	glm::vec4 color;
	float homingStrength;
	float timer = 0.0f;

public:
	AimedShotEmitter(float bulletSpeed, float interval, glm::vec4 color, float homingStrength = 0.0f);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition) override;
};

}
