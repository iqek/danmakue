#pragma once

#include <glm/glm.hpp>

namespace Engine {

class BulletPool;

// Base interface for a bullet-spawning pattern.
// update them all identically
class BulletEmitter {
public:
	virtual ~BulletEmitter() = default;

	virtual void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 targetPosition) = 0;
};

}
