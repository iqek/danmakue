#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <vector>

#include "engine/danmaku/MovementPattern.h"

namespace Engine {

struct Waypoint {
	glm::vec2 position{ 0.0f, 0.0f };
	float waitTime = 0.0f;
};

// Moves toward each waypoint in turn at a constant speed, pausing for that
// waypoint's waitTime once reached before continuing to the next one, and
// holds position at the last one indefinitely once the path is complete.
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
