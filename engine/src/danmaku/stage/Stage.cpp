#include "engine/danmaku/stage/Stage.h"
#include "engine/danmaku/Enemy.h"

namespace Engine {

void Stage::AddEvent(StageEvent event){
	events.push_back(std::move(event));
}

void Stage::Update(entt::registry& registry, float deltaTime){
	timeSinceLastEvent += deltaTime;

	// while, not if, so any number of zero-delay events share one instant
	while(nextEvent < events.size() && events[nextEvent].trigger(registry, timeSinceLastEvent)){
		events[nextEvent].spawn(registry);
		nextEvent++;
		timeSinceLastEvent = 0.0f;
	}
}

std::function<bool(entt::registry&, float)> DelayTrigger(float seconds){
	return [seconds](entt::registry&, float timeSinceLastEvent){
		return timeSinceLastEvent >= seconds;
	};
}

std::function<bool(entt::registry&, float)> AfterEnemiesCleared(){
	return [](entt::registry& registry, float){
		return registry.view<Enemy>().empty();
	};
}

}
