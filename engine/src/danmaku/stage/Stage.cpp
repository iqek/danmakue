#include "engine/danmaku/stage/Stage.h"
#include "engine/danmaku/Enemy.h"

namespace Engine {

void Stage::AddEvent(StageEvent event){
	events.push_back(std::move(event));
}

void Stage::Update(entt::registry& registry, float deltaTime){
	elapsedTime += deltaTime;

	// while, not if, in case a frame hitch makes more than one event ready
	while(nextEvent < events.size() && events[nextEvent].trigger(registry, elapsedTime)){
		events[nextEvent].spawn(registry);
		nextEvent++;
	}
}

std::function<bool(entt::registry&, float)> TimeTrigger(float seconds){
	return [seconds](entt::registry&, float elapsedTime){
		return elapsedTime >= seconds;
	};
}

std::function<bool(entt::registry&, float)> AfterEnemiesCleared(){
	return [](entt::registry& registry, float){
		return registry.view<Enemy>().empty();
	};
}

}
