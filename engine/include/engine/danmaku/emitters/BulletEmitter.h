#pragma once

#include <glm/glm.hpp>

namespace Engine {

class BulletPool;

// Base interface for a bullet-spawning pattern.
// originPosition/targetPosition are passed in fresh each frame, not stored.
class BulletEmitter {
public:
	virtual ~BulletEmitter() = default;

	virtual void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition) = 0;
};

}
