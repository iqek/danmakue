#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/MovementPattern.h"

namespace Engine {

// Moves at a constant velocity forever - e.g. an enemy flying straight
// across or down the screen.
class LinearMovement : public MovementPattern {
private:
	glm::vec2 velocity;

public:
	explicit LinearMovement(glm::vec2 velocity);

	void Update(float deltaTime, Transform& transform) override;
};

}
