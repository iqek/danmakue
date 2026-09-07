#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Editor {

// An enemy template: everything about it except where/when it spawns
struct EnemyDefinition {
	std::string id;
	glm::vec2 size{ 40.0f, 40.0f };
	glm::vec2 colliderSize{ 36.0f, 36.0f };
	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	int health = 1;
	nlohmann::json movement = nlohmann::json::object();
	nlohmann::json emitters = nlohmann::json::array();
};

// A single spawn event: when, what, and where
struct TimelineEntry {
	nlohmann::json trigger = nlohmann::json::object();
	std::string spawnId;
	glm::vec2 position{ 0.0f, 0.0f };
};

// The stage's single player: spawn, stats, and starting weapons
struct PlayerDefinition {
	glm::vec2 position{ 640.0f, 360.0f };
	glm::vec2 size{ 80.0f, 80.0f };
	glm::vec2 colliderSize{ 30.0f, 30.0f };
	glm::vec4 color{ 1.0f, 0.55f, 0.65f, 1.0f };
	int lives = 3;
	float moveSpeed = 300.0f;
	nlohmann::json weapons = nlohmann::json::array();
};

struct StageDefinition {
	PlayerDefinition player;
	std::vector<EnemyDefinition> enemies;
	std::vector<TimelineEntry> timeline;
};

// Both throw nlohmann::json::exception on malformed input
StageDefinition LoadStageDefinition(const std::string& utf8Path);
void SaveStageDefinition(const StageDefinition& stage, const std::string& utf8Path);

}
