#include "StageDefinition.h"

#include "engine/core/Utf8Path.h"
#include "JsonFloat.h"

#include <algorithm>
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

// enemies used to be a fixed set of fields; fold an old one into the component list
Json MigrateEnemyComponents(const Json& enemyData, const glm::vec2& size){
	Json components = Json::array();

	components.push_back(Json{
		{ "type", "sprite" },
		{ "color", enemyData.contains("color") ? enemyData.at("color") : Json::array({ 1.0, 1.0, 1.0, 1.0 }) }
	});

	components.push_back(Json{
		{ "type", "collider" },
		{ "size", enemyData.contains("colliderSize") ? enemyData.at("colliderSize") : ToJsonArray(size) }
	});

	components.push_back(Json{ { "type", "health" }, { "hp", enemyData.value("health", 1) } });

	if(enemyData.contains("movement")){
		components.push_back(Json{ { "type", "movement" }, { "pattern", enemyData.at("movement") } });
	}
	if(enemyData.contains("emitters")){
		components.push_back(Json{ { "type", "emitters" }, { "list", enemyData.at("emitters") } });
	}

	return components;
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

	if(data.contains("phases")){
		for(const auto& phase : data.at("phases")){
			PhaseDefinition definition;
			// phases used to be written as plain names, so accept both shapes
			if(phase.is_string()){
				definition.name = phase.get<std::string>();
			}
			else{
				definition.name = phase.value("name", std::string());
				if(phase.contains("color")){
					definition.color = ParseColor(phase.at("color"));
					stage.phases.push_back(definition);
					continue;
				}
			}
			definition.color = DefaultPhaseColor(definition.name);
			stage.phases.push_back(definition);
		}
	}

	for(const auto& enemyData : data.at("enemies")){
		EnemyDefinition enemy;
		enemy.id = enemyData.at("id").get<std::string>();
		enemy.phase = enemyData.value("phase", std::string());
		enemy.size = ParseVec2(enemyData.at("size"));
		enemy.components = enemyData.contains("components") ? enemyData.at("components") : MigrateEnemyComponents(enemyData, enemy.size);
		stage.enemies.push_back(std::move(enemy));
	}

	for(const auto& entry : data.at("timeline")){
		TimelineEntry timelineEntry;
		timelineEntry.trigger = entry.at("trigger");
		timelineEntry.spawnId = entry.at("spawn").get<std::string>();
		timelineEntry.position = ParseVec2(entry.at("position"));

		// spawns used to carry their own phase, so lift any old one onto its enemy
		std::string legacyPhase = entry.value("phase", std::string());
		if(!legacyPhase.empty()){
			for(auto& enemy : stage.enemies){
				if(enemy.id == timelineEntry.spawnId && enemy.phase.empty()){
					enemy.phase = legacyPhase;
				}
			}
		}

		stage.timeline.push_back(std::move(timelineEntry));
	}

	// waypoints used to be absolute world points, which made a template unusable at a
	// second spawn. Fold any old ones into offsets from where that enemy first spawns.
	for(auto& enemy : stage.enemies){
		Json* pattern = FindMovementPattern(enemy);
		if(pattern == nullptr || pattern->value("type", std::string()) != "waypoint" || !pattern->contains("waypoints")){
			continue;
		}

		glm::vec2 anchor(0.0f);
		for(const auto& entry : stage.timeline){
			if(entry.spawnId == enemy.id){
				anchor = entry.position;
				break;
			}
		}

		for(auto& point : pattern->at("waypoints")){
			if(point.contains("offset") || !point.contains("position")){
				continue;
			}
			glm::vec2 absolute = ParseVec2(point.at("position"));
			point.erase("position");
			point["offset"] = ToJsonArray(absolute - anchor);
		}
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

	// optional keys are only written when actually set, so untouched stages
	// don't grow empty "phase" fields all over their diffs
	if(!stage.phases.empty()){
		data["phases"] = Json::array();
		for(const auto& phase : stage.phases){
			Json phaseData;
			phaseData["name"] = phase.name;
			phaseData["color"] = ToJsonArray(phase.color);
			data["phases"].push_back(phaseData);
		}
	}

	data["enemies"] = Json::array();
	for(const auto& enemy : stage.enemies){
		Json enemyData;
		enemyData["id"] = enemy.id;
		if(!enemy.phase.empty()){
			enemyData["phase"] = enemy.phase;
		}
		enemyData["size"] = ToJsonArray(enemy.size);
		enemyData["components"] = enemy.components;
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

const EnemyDefinition* FindEnemy(const StageDefinition& stage, const std::string& id){
	for(const auto& enemy : stage.enemies){
		if(enemy.id == id){
			return &enemy;
		}
	}
	return nullptr;
}

const std::string& EntryPhase(const StageDefinition& stage, const TimelineEntry& entry){
	static const std::string none;
	const EnemyDefinition* enemy = FindEnemy(stage, entry.spawnId);
	return enemy ? enemy->phase : none;
}

Json* FindComponent(Json& components, const std::string& type){
	if(!components.is_array()){
		return nullptr;
	}
	for(auto& component : components){
		if(component.value("type", std::string()) == type){
			return &component;
		}
	}
	return nullptr;
}

const Json* FindComponent(const Json& components, const std::string& type){
	return FindComponent(const_cast<Json&>(components), type);
}

Json* FindMovementPattern(EnemyDefinition& enemy){
	Json* movement = FindComponent(enemy.components, "movement");
	if(movement == nullptr || !movement->contains("pattern")){
		return nullptr;
	}
	return &movement->at("pattern");
}

int WaypointCount(const Json& movement){
	if(!movement.contains("waypoints") || !movement.at("waypoints").is_array()){
		return 0;
	}
	return static_cast<int>(movement.at("waypoints").size());
}

void InsertWaypointAfter(Json& movement, int index){
	if(!movement.contains("waypoints") || !movement.at("waypoints").is_array()){
		movement["waypoints"] = Json::array();
	}

	Json& points = movement["waypoints"];
	int count = static_cast<int>(points.size());
	index = std::max(-1, std::min(index, count - 1));

	auto offsetAt = [&](int i){
		return (i >= 0 && i < count && points[i].contains("offset")) ? ParseVec2(points[i].at("offset")) : glm::vec2(0.0f);
	};

	glm::vec2 here = offsetAt(index);
	// halfway to the next point, or simply beyond it when there is nothing after
	glm::vec2 placed = index + 1 < count ? (here + offsetAt(index + 1)) * 0.5f : here + glm::vec2(80.0f, 0.0f);

	Json point;
	point["offset"] = ToJsonArray(placed);
	point["waitTime"] = 0.0;
	points.insert(points.begin() + (index + 1), point);
}

void RemoveWaypoint(Json& movement, int index){
	if(!movement.contains("waypoints") || !movement.at("waypoints").is_array()){
		return;
	}

	Json& points = movement["waypoints"];
	if(index < 0 || index >= static_cast<int>(points.size())){
		return;
	}
	points.erase(points.begin() + index);
}

const PhaseDefinition* FindPhase(const StageDefinition& stage, const std::string& name){
	for(const auto& phase : stage.phases){
		if(phase.name == name){
			return &phase;
		}
	}
	return nullptr;
}

glm::vec4 DefaultPhaseColor(const std::string& name){
	// hand-picked and light, so dark labels stay readable on top of them
	static const glm::vec4 palette[] = {
		{ 0.96f, 0.78f, 0.42f, 1.0f },
		{ 0.55f, 0.82f, 0.96f, 1.0f },
		{ 0.74f, 0.90f, 0.60f, 1.0f },
		{ 0.96f, 0.66f, 0.76f, 1.0f },
		{ 0.80f, 0.73f, 0.96f, 1.0f },
		{ 0.58f, 0.90f, 0.83f, 1.0f },
	};

	unsigned int hash = 2166136261u;
	for(char c : name){
		hash = (hash ^ static_cast<unsigned char>(c)) * 16777619u;
	}
	return palette[hash % (sizeof(palette) / sizeof(palette[0]))];
}

glm::vec4 PhaseColorOf(const StageDefinition& stage, const std::string& name){
	const PhaseDefinition* phase = FindPhase(stage, name);
	return phase ? phase->color : DefaultPhaseColor(name);
}

}
