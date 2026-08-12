#include "engine/danmaku/WaypointMovement.h"
#include "engine/scene/Components.h"

namespace Engine {

WaypointMovement::WaypointMovement(std::vector<Waypoint> waypoints, float speed):
	waypoints(std::move(waypoints)), speed(speed)
{
}

void WaypointMovement::Update(float deltaTime, Transform& transform){
	if(currentWaypoint >= waypoints.size()){
		return;
	}

	if(waitTimer > 0.0f){
		waitTimer -= deltaTime;
		return;
	}

	glm::vec2 target = waypoints[currentWaypoint].position;
	glm::vec2 toTarget = target - transform.position;
	float distance = glm::length(toTarget);

	float step = speed * deltaTime;
	if(step >= distance){
		transform.position = target;
		waitTimer = waypoints[currentWaypoint].waitTime;
		currentWaypoint++;
		return;
	}

	transform.position += (toTarget / distance) * step;
}

}
