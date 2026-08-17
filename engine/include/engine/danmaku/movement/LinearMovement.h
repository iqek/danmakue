#pragma once

#include <glm/glm.hpp>

#include "engine/danmaku/movement/MovementPattern.h"

namespace Engine {

// Moves at a constant velocity forever
class LinearMovement : public MovementPattern {
private:
	glm::vec2 velocity;

public:
	explicit LinearMovement(glm::vec2 velocity);

	void Update(float deltaTime, Transform& transform) override;
};

}
