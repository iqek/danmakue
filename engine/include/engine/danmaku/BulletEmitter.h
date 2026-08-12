#pragma once

#include <glm/glm.hpp>

namespace Engine {

class BulletPool;

// Base interface for a bullet-spawning pattern. originPosition is supplied
// fresh every frame (usually the owning entity's current Transform.position)
// rather than stored, so a moving enemy keeps firing from wherever it
// currently is. targetPosition (usually the player) is passed to every
// emitter too, even ones that ignore it, so a boss can hold a list of mixed
// emitter types and update them all identically.
class BulletEmitter {
public:
	virtual ~BulletEmitter() = default;

	virtual void Update(float deltaTime, BulletPool& bulletPool, glm::vec2 originPosition, glm::vec2 targetPosition) = 0;
};

}
