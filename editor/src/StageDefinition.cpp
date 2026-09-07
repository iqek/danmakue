#include "StageDefinition.h"

#include "engine/core/Utf8Path.h"
#include "JsonFloat.h"

#include <fstream>

namespace Editor {

namespace {

using Json = nlohmann::json;

glm::vec2 ParseVec2(const Json& array){
	return glm::vec2(array.at(0).get<float>(), array.at(1).get<float>());
}

glm::vec4 ParseColor(const Json& array){
	return glm::vec4(array.at(0).get<float>(), array.at(1).get<float>(), array.at(2).get<float>(), array.at(3).get<float>());
}

Json ToJsonArray(const glm::vec2& v){
	return Json::array({ CleanFloat(v.x), CleanFloat(v.y) });
}

Json ToJsonArray(const glm::vec4& v){
	return Json::array({ CleanFloat(v.x), CleanFloat(v.y), CleanFloat(v.z), CleanFloat(v.w) });
}

}

StageDefinition LoadStageDefinition(const std::string& utf8Path){
	std::ifstream file(Engine::PathFromUtf8(utf8Path));
	if(!file){
		throw std::runtime_error("Failed to open stage file: " + utf8Path);
	}

	Json data;
	file >> data;

	StageDefinition stage;

	if(data.contains("player")){
		const Json& playerData = data.at("player");
		stage.player.position = playerData.contains("position") ? ParseVec2(playerData.at("position")) : stage.player.position;
		stage.player.size = playerData.contains("size") ? ParseVec2(playerData.at("size")) : stage.player.size;
		stage.player.colliderSize = playerData.contains("colliderSize") ? ParseVec2(playerData.at("colliderSize")) : stage.player.size;
		stage.player.color = playerData.contains("color") ? ParseColor(playerData.at("color")) : stage.player.color;
		stage.player.lives = playerData.value("lives", stage.player.lives);
		stage.player.moveSpeed = playerData.value("moveSpeed", stage.player.moveSpeed);
		stage.player.weapons = playerData.value("weapons", Json::array());
	}

	for(const auto& enemyData : data.at("enemies")){
		EnemyDefinition enemy;
		enemy.id = enemyData.at("id").get<std::string>();
		enemy.size = ParseVec2(enemyData.at("size"));
		enemy.colliderSize = enemyData.contains("colliderSize") ? ParseVec2(enemyData.at("colliderSize")) : enemy.size;
		enemy.color = ParseColor(enemyData.at("color"));
		enemy.health = enemyData.value("health", 1);
		enemy.movement = enemyData.value("movement", Json::object());
		enemy.emitters = enemyData.value("emitters", Json::array());
		stage.enemies.push_back(std::move(enemy));
	}

	for(const auto& entry : data.at("timeline")){
		TimelineEntry timelineEntry;
		timelineEntry.trigger = entry.at("trigger");
		timelineEntry.spawnId = entry.at("spawn").get<std::string>();
		timelineEntry.position = ParseVec2(entry.at("position"));
		stage.timeline.push_back(std::move(timelineEntry));
	}

	return stage;
}

void SaveStageDefinition(const StageDefinition& stage, const std::string& utf8Path){
	Json data;

	Json playerData;
	playerData["position"] = ToJsonArray(stage.player.position);
	playerData["size"] = ToJsonArray(stage.player.size);
	playerData["colliderSize"] = ToJsonArray(stage.player.colliderSize);
	playerData["color"] = ToJsonArray(stage.player.color);
	playerData["lives"] = stage.player.lives;
	playerData["moveSpeed"] = CleanFloat(stage.player.moveSpeed);
	playerData["weapons"] = stage.player.weapons;
	data["player"] = playerData;

	data["enemies"] = Json::array();
	for(const auto& enemy : stage.enemies){
		Json enemyData;
		enemyData["id"] = enemy.id;
		enemyData["size"] = ToJsonArray(enemy.size);
		enemyData["colliderSize"] = ToJsonArray(enemy.colliderSize);
		enemyData["color"] = ToJsonArray(enemy.color);
		enemyData["health"] = enemy.health;
		enemyData["movement"] = enemy.movement;
		enemyData["emitters"] = enemy.emitters;
		data["enemies"].push_back(enemyData);
	}

	data["timeline"] = Json::array();
	for(const auto& entry : stage.timeline){
		Json entryData;
		entryData["trigger"] = entry.trigger;
		entryData["spawn"] = entry.spawnId;
		entryData["position"] = ToJsonArray(entry.position);
		data["timeline"].push_back(entryData);
	}

	std::ofstream file(Engine::PathFromUtf8(utf8Path));
	if(!file){
		throw std::runtime_error("Failed to open stage file for writing: " + utf8Path);
	}
	file << data.dump(4);
}

}
