#pragma once

#include <cstddef>
#include <entt/entt.hpp>
#include <functional>
#include <vector>

namespace Engine {

struct StageEvent {
	std::function<bool(entt::registry&, float elapsedTime)> trigger;
	std::function<void(entt::registry&)> spawn;
};

// An ordered timeline of spawn events; event N waits until N-1 has fired.
class Stage {
private:
	std::vector<StageEvent> events;
	std::size_t nextEvent = 0;
	float elapsedTime = 0.0f;

public:
	void AddEvent(StageEvent event);
	void Update(entt::registry& registry, float deltaTime);
};

// Common trigger builders, so callers don't have to write raw lambdas
std::function<bool(entt::registry&, float)> TimeTrigger(float seconds);
std::function<bool(entt::registry&, float)> AfterEnemiesCleared();

}
