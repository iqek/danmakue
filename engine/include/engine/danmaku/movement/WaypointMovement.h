#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <vector>

#include "engine/danmaku/movement/MovementPattern.h"

namespace Engine {

struct Waypoint {
	glm::vec2 position{ 0.0f, 0.0f };
	float waitTime = 0.0f;
};

// Moves through each waypoint in turn, pausing waitTime at each one
class WaypointMovement : public MovementPattern {
private:
	std::vector<Waypoint> waypoints;
	float speed;
	std::size_t currentWaypoint = 0;
	float waitTimer = 0.0f;

public:
	WaypointMovement(std::vector<Waypoint> waypoints, float speed);

	void Update(float deltaTime, Transform& transform) override;
};

}
