#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <vector>

#include "engine/danmaku/movement/MovementPattern.h"

namespace Engine {

// Measured from wherever the enemy spawned, so one path can be reused anywhere
struct Waypoint {
	glm::vec2 offset{ 0.0f, 0.0f };
	float waitTime = 0.0f;
};

// Moves through each waypoint in turn, pausing waitTime at each one
class WaypointMovement : public MovementPattern {
private:
	std::vector<Waypoint> waypoints;
	float speed;
	glm::vec2 origin;
	std::size_t currentWaypoint = 0;
	float waitTimer = 0.0f;

public:
	WaypointMovement(std::vector<Waypoint> waypoints, float speed, glm::vec2 origin);

	void Update(float deltaTime, Transform& transform) override;
};

}
