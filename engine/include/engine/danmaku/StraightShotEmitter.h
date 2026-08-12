#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/BulletEmitter.h"

namespace Engine {

// Fires one bullet in a fixed direction every interval seconds - the
// simplest possible pattern, and the natural default for a player's own shot.
class StraightShotEmitter : public BulletEmitter {
private:
	glm::vec2 direction;
	float bulletSpeed;
	float interval;
	glm::vec4 color;
	float timer = 0.0f;

public:
	StraightShotEmitter(glm::vec2 direction, float bulletSpeed, float interval, glm::vec4 color);

	void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition) override;
};

}
