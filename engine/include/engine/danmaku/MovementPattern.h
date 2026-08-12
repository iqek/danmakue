#pragma once

namespace Engine {

struct Transform;

// Base interface for how an enemy moves. Update() is called once per frame
// and is free to change the given Transform however it likes - this is
// architected the same way as BulletEmitter specifically so a future scripted
// movement pattern slots in as just one more implementation of this interface.
class MovementPattern {
public:
	virtual ~MovementPattern() = default;

	virtual void Update(float deltaTime, Transform& transform) = 0;
};

}
