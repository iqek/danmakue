#pragma once

#include <cstddef>
#include <entt/entt.hpp>
#include <functional>
#include <vector>

namespace Engine {

struct StageEvent {
	// the float is seconds since the previous event fired, not since the stage began
	std::function<bool(entt::registry&, float)> trigger;
	std::function<void(entt::registry&)> spawn;
};

// An ordered timeline of spawn events; event N waits until N-1 has fired.
class Stage {
private:
	std::vector<StageEvent> events;
	std::size_t nextEvent = 0;
	float timeSinceLastEvent = 0.0f;

public:
	void AddEvent(StageEvent event);
	void Update(entt::registry& registry, float deltaTime);
};

// Fires once that many seconds have passed since the previous event.
// A delay of zero spawns in the same instant as the event before it.
std::function<bool(entt::registry&, float)> DelayTrigger(float seconds);

// Fires once nothing is left alive on screen.
std::function<bool(entt::registry&, float)> AfterEnemiesCleared();

}
