#include "engine/danmaku/stage/StageLoader.h"
#include "engine/danmaku/Enemy.h"
#include "engine/danmaku/emitters/AimedShotEmitter.h"
#include "engine/danmaku/emitters/RadialBurstEmitter.h"
#include "engine/danmaku/emitters/RandomScatterEmitter.h"
#include "engine/danmaku/emitters/SpiralEmitter.h"
#include "engine/danmaku/emitters/StraightShotEmitter.h"
#include "engine/danmaku/movement/LinearMovement.h"
#include "engine/danmaku/movement/WaypointMovement.h"
#include "engine/scene/Components.h"
#include "engine/core/Log.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace Engine {

namespace {

using Json = nlohmann::json;

glm::vec2 ParseVec2(const Json& array){
	return glm::vec2(array.at(0).get<float>(), array.at(1).get<float>());
}

glm::vec4 ParseColor(const Json& array){
	return glm::vec4(array.at(0).get<float>(), array.at(1).get<float>(), array.at(2).get<float>(), array.at(3).get<float>());
}

std::unique_ptr<MovementPattern> CreateMovement(const Json& data){
	std::string type = data.at("type").get<std::string>();

	if(type == "linear"){
		return std::make_unique<LinearMovement>(ParseVec2(data.at("velocity")));
	}

	if(type == "waypoint"){
		std::vector<Waypoint> waypoints;
		for(const auto& point : data.at("waypoints")){
			waypoints.push_back({ ParseVec2(point.at("position")), point.value("waitTime", 0.0f) });
		}
		return std::make_unique<WaypointMovement>(waypoints, data.at("speed").get<float>());
	}

	ENGINE_CORE_ERROR("Unknown movement type: {}", type);
	return nullptr;
}

std::unique_ptr<BulletEmitter> CreateEmitter(const Json& data){
	std::string type = data.at("type").get<std::string>();
	glm::vec4 color = ParseColor(data.at("color"));

	if(type == "radialBurst"){
		return std::make_unique<RadialBurstEmitter>(data.at("bulletCount").get<int>(), data.at("bulletSpeed").get<float>(), data.at("interval").get<float>(), color);
	}
	if(type == "spiral"){
		return std::make_unique<SpiralEmitter>(data.at("bulletSpeed").get<float>(), data.at("spawnInterval").get<float>(), data.at("angularVelocity").get<float>(), data.at("arms").get<int>(), color);
	}
	if(type == "aimedShot"){
		return std::make_unique<AimedShotEmitter>(data.at("bulletSpeed").get<float>(), data.at("interval").get<float>(), color, data.value("homingStrength", 0.0f));
	}
	if(type == "randomScatter"){
		return std::make_unique<RandomScatterEmitter>(data.at("minSpeed").get<float>(), data.at("maxSpeed").get<float>(), data.at("spawnInterval").get<float>(), color, data.value("outwardAcceleration", 0.0f));
	}
	if(type == "straightShot"){
		return std::make_unique<StraightShotEmitter>(ParseVec2(data.at("direction")), data.at("bulletSpeed").get<float>(), data.at("interval").get<float>(), color);
	}

	ENGINE_CORE_ERROR("Unknown emitter type: {}", type);
	return nullptr;
}

std::function<bool(entt::registry&, float)> CreateTrigger(const Json& data){
	std::string type = data.at("type").get<std::string>();

	if(type == "time"){
		return TimeTrigger(data.at("seconds").get<float>());
	}
	if(type == "afterCleared"){
		return AfterEnemiesCleared();
	}

	ENGINE_CORE_ERROR("Unknown trigger type: {}", type);
	return TimeTrigger(0.0f);
}

void SpawnEnemy(entt::registry& registry, const Json& enemyData){
	glm::vec2 position = ParseVec2(enemyData.at("position"));
	glm::vec2 size = ParseVec2(enemyData.at("size"));
	glm::vec2 colliderSize = enemyData.contains("colliderSize") ? ParseVec2(enemyData.at("colliderSize")) : size;
	glm::vec4 color = ParseColor(enemyData.at("color"));

	auto entity = registry.create();
	registry.emplace<Transform>(entity, position, size);
	registry.emplace<Sprite>(entity, color);
	registry.emplace<Collider>(entity, colliderSize);

	Enemy enemy;
	enemy.health = enemyData.value("health", 1);
	enemy.maxHealth = enemy.health;

	if(enemyData.contains("movement")){
		enemy.movement = CreateMovement(enemyData.at("movement"));
	}

	for(const auto& emitterData : enemyData.at("emitters")){
		enemy.emitters.push_back(CreateEmitter(emitterData));
	}

	registry.emplace<Enemy>(entity, std::move(enemy));
}

}

Stage LoadStage(const std::string& path){
	std::ifstream file(path);
	if(!file){
		ENGINE_CORE_ERROR("Failed to open stage file: {}", path);
		return Stage{};
	}

	Stage stage;

	try{
		Json data;
		file >> data;

		for(const auto& enemyData : data.at("enemies")){
			auto trigger = CreateTrigger(enemyData.at("trigger"));

			// enemyData is captured by value - it's a plain copyable json
			// object, so this stays valid long after the file is closed.
			auto spawn = [enemyData](entt::registry& registry){
				try{
					SpawnEnemy(registry, enemyData);
				}
				catch(const Json::exception& e){
					ENGINE_CORE_ERROR("Failed to spawn enemy: {}", e.what());
				}
			};

			stage.AddEvent({ trigger, spawn });
		}
	}
	catch(const Json::exception& e){
		ENGINE_CORE_ERROR("Failed to parse stage file {}: {}", path, e.what());
	}

	return stage;
}

}
