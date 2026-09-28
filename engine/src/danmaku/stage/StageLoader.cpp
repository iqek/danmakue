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
#include "engine/render/TextureLibrary.h"
#include "engine/scene/Components.h"
#include "engine/core/Log.h"
#include "engine/core/Utf8Path.h"

#include <algorithm>
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

// a collider component, falling back to a box the size of the entity itself
Collider ParseCollider(const Json& data, const glm::vec2& fallbackSize){
	Collider collider;

	if(data.value("shape", std::string("box")) == "circle"){
		collider.shape = Collider::Shape::Circle;
		collider.radius = data.value("radius", std::max(fallbackSize.x, fallbackSize.y) * 0.5f);
		return collider;
	}

	collider.size = data.contains("size") ? ParseVec2(data.at("size")) : fallbackSize;
	return collider;
}

std::unique_ptr<MovementPattern> CreateMovement(const Json& data, const glm::vec2& spawnPosition){
	std::string type = data.at("type").get<std::string>();

	if(type == "linear"){
		return std::make_unique<LinearMovement>(ParseVec2(data.at("velocity")));
	}

	if(type == "waypoint"){
		std::vector<Waypoint> waypoints;
		for(const auto& point : data.at("waypoints")){
			// waypoints used to be absolute world points, so fold an old one back into an offset
			glm::vec2 offset = point.contains("offset") ? ParseVec2(point.at("offset")) : ParseVec2(point.at("position")) - spawnPosition;
			waypoints.push_back({ offset, point.value("waitTime", 0.0f) });
		}
		return std::make_unique<WaypointMovement>(waypoints, data.at("speed").get<float>(), spawnPosition);
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

	if(type == "delay"){
		return DelayTrigger(data.value("seconds", 0.0f));
	}
	if(type == "afterCleared"){
		return AfterEnemiesCleared();
	}

	ENGINE_CORE_ERROR("Unknown trigger type: {}", type);
	return DelayTrigger(0.0f);
}

const Json* FindComponent(const Json& enemyData, const char* type){
	if(!enemyData.contains("components") || !enemyData.at("components").is_array()){
		return nullptr;
	}
	for(const auto& component : enemyData.at("components")){
		if(component.value("type", std::string()) == type){
			return &component;
		}
	}
	return nullptr;
}

// An enemy only gets what its template actually lists, the way a component list should behave.
// assetsRoot is what texture paths in the stage are relative to.
void SpawnEnemy(entt::registry& registry, const Json& enemyData, const glm::vec2& position, const std::string& assetsRoot){
	glm::vec2 size = ParseVec2(enemyData.at("size"));

	auto entity = registry.create();
	registry.emplace<Transform>(entity, position, size);

	if(const Json* sprite = FindComponent(enemyData, "sprite")){
		glm::vec4 color = sprite->contains("color") ? ParseColor(sprite->at("color")) : glm::vec4(1.0f);
		std::string texture = sprite->value("texture", std::string());
		registry.emplace<Sprite>(entity, color, texture.empty() ? nullptr : TextureLibrary::Get(assetsRoot + "/" + texture));
	}

	if(const Json* collider = FindComponent(enemyData, "collider")){
		registry.emplace<Collider>(entity, ParseCollider(*collider, size));
	}

	Enemy enemy;
	if(const Json* health = FindComponent(enemyData, "health")){
		enemy.health = health->value("hp", 1);
	}
	enemy.maxHealth = enemy.health;

	if(const Json* movement = FindComponent(enemyData, "movement")){
		if(movement->contains("pattern")){
			enemy.movement = CreateMovement(movement->at("pattern"), position);
		}
	}

	if(const Json* emitters = FindComponent(enemyData, "emitters")){
		if(emitters->contains("list")){
			for(const auto& emitterData : emitters->at("list")){
				enemy.emitters.push_back(CreateEmitter(emitterData));
			}
		}
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

	// a stage's texture paths are relative to the assets folder, which is the
	// stages folder's parent - both the game and the editor pass a full path in
	std::string assetsRoot = Utf8FromPath(PathFromUtf8(path).parent_path().parent_path());

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
			auto spawn = [enemyData, position, assetsRoot](entt::registry& registry){
				try{
					SpawnEnemy(registry, enemyData, position, assetsRoot);
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
	std::string assetsRoot = Utf8FromPath(PathFromUtf8(path).parent_path().parent_path());

	glm::vec2 position = playerData.contains("position") ? ParseVec2(playerData.at("position")) : glm::vec2(640.0f, 360.0f);
	glm::vec2 size = playerData.contains("size") ? ParseVec2(playerData.at("size")) : glm::vec2(80.0f, 80.0f);

	auto entity = registry.create();
	registry.emplace<Transform>(entity, position, size);

	// like an enemy, the player only gets what its components actually list
	if(const Json* sprite = FindComponent(playerData, "sprite")){
		glm::vec4 color = sprite->contains("color") ? ParseColor(sprite->at("color")) : glm::vec4(1.0f);
		std::string texture = sprite->value("texture", std::string());
		registry.emplace<Sprite>(entity, color, texture.empty() ? nullptr : TextureLibrary::Get(assetsRoot + "/" + texture));
	}

	if(const Json* collider = FindComponent(playerData, "collider")){
		registry.emplace<Collider>(entity, ParseCollider(*collider, size));
	}

	Player player;
	if(const Json* settings = FindComponent(playerData, "player")){
		player.lives = settings->value("lives", 3);
		player.moveSpeed = settings->value("moveSpeed", 300.0f);
		player.invincibleDuration = settings->value("invincibleSeconds", 1.5f);
	}

	if(const Json* weapons = FindComponent(playerData, "weapons")){
		for(const auto& weaponData : weapons->value("list", Json::array())){
			try{
				player.weapons.push_back(CreateEmitter(weaponData));
			}
			catch(const Json::exception& e){
				ENGINE_CORE_ERROR("Failed to create player weapon: {}", e.what());
			}
		}
	}

	registry.emplace<Player>(entity, std::move(player));
	return entity;
}

}
