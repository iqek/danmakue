#pragma once

namespace Engine {

struct Transform;

// Base interface for how an enemy moves, one Update() call per frame.
// Shaped like BulletEmitter so a scripted movement fits in the same way later.
class MovementPattern {
public:
	virtual ~MovementPattern() = default;

	virtual void Update(float deltaTime, Transform& transform) = 0;
};

}
