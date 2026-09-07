#include "engine/danmaku/stage/StageLoader.h"
#include "engine/danmaku/Enemy.h"
#include "engine/danmaku/Player.h"
#include "engine/danmaku/emitters/AimedShotEmitter.h"
#include "engine/danmaku/emitters/RadialBurstEmitter.h"
#include "engine/danmaku/emitters/RandomScatterEmitter.h"
#include "engine/danmaku/emitters/SpiralEmitter.h"
#include "engine/danmaku/emitters/StraightShotEmitter.h"
#include "engine/danmaku/movement/LinearMovement.h"
#include "engine/danmaku/movement/WaypointMovement.h"
#include "engine/scene/Components.h"
#include "engine/core/Log.h"
#include "engine/core/Utf8Path.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>

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

void SpawnEnemy(entt::registry& registry, const Json& enemyData, const glm::vec2& position){
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
	std::ifstream file(PathFromUtf8(path));
	if(!file){
		ENGINE_CORE_ERROR("Failed to open stage file: {}", path);
		return Stage{};
	}

	Stage stage;

	try{
		Json data;
		file >> data;

		// enemies are reusable templates, keyed by id and referenced from the timeline
		std::unordered_map<std::string, Json> enemyTemplates;
		for(const auto& enemyData : data.at("enemies")){
			enemyTemplates[enemyData.at("id").get<std::string>()] = enemyData;
		}

		for(const auto& entry : data.at("timeline")){
			std::string spawnId = entry.at("spawn").get<std::string>();
			auto templateIt = enemyTemplates.find(spawnId);
			if(templateIt == enemyTemplates.end()){
				ENGINE_CORE_ERROR("Timeline entry references unknown enemy id: {}", spawnId);
				continue;
			}

			auto trigger = CreateTrigger(entry.at("trigger"));
			Json enemyData = templateIt->second;
			glm::vec2 position = ParseVec2(entry.at("position"));

			// enemyData is captured by value - it's a plain copyable json
			// object, so this stays valid long after the file is closed.
			auto spawn = [enemyData, position](entt::registry& registry){
				try{
					SpawnEnemy(registry, enemyData, position);
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

entt::entity SpawnPlayer(entt::registry& registry, const std::string& path){
	// unlike LoadStage/SpawnEnemy, this always has to return a usable entity -
	// the game loop dereferences the player unconditionally, so falling back
	// to defaults here (even on a missing/unreadable file) beats returning
	// entt::null and crashing the very first frame
	Json data = Json::object();

	std::ifstream file(PathFromUtf8(path));
	if(!file){
		ENGINE_CORE_ERROR("Failed to open stage file: {}", path);
	}
	else{
		try{
			file >> data;
		}
		catch(const Json::exception& e){
			ENGINE_CORE_ERROR("Failed to parse stage file {}: {}", path, e.what());
		}
	}

	Json playerData = data.value("player", Json::object());

	glm::vec2 position = playerData.contains("position") ? ParseVec2(playerData.at("position")) : glm::vec2(640.0f, 360.0f);
	glm::vec2 size = playerData.contains("size") ? ParseVec2(playerData.at("size")) : glm::vec2(80.0f, 80.0f);
	glm::vec2 colliderSize = playerData.contains("colliderSize") ? ParseVec2(playerData.at("colliderSize")) : glm::vec2(30.0f, 30.0f);
	glm::vec4 color = playerData.contains("color") ? ParseColor(playerData.at("color")) : glm::vec4(1.0f, 0.55f, 0.65f, 1.0f);

	auto entity = registry.create();
	registry.emplace<Transform>(entity, position, size);
	registry.emplace<Sprite>(entity, color);
	registry.emplace<Collider>(entity, colliderSize);

	Player player;
	player.lives = playerData.value("lives", 3);
	player.moveSpeed = playerData.value("moveSpeed", 300.0f);

	if(playerData.contains("weapons")){
		for(const auto& weaponData : playerData.at("weapons")){
			try{
				player.weapons.push_back(CreateEmitter(weaponData));
			}
			catch(const Json::exception& e){
				ENGINE_CORE_ERROR("Failed to create player weapon: {}", e.what());
			}
		}
	}
	else{
		player.weapons.push_back(std::make_unique<StraightShotEmitter>(glm::vec2(0.0f, -1.0f), 500.0f, 0.12f, color));
	}

	registry.emplace<Player>(entity, std::move(player));
	return entity;
}

}
