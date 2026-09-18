#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Editor {

// An enemy template: everything about it except where/when it spawns.
// Size is intrinsic the way Unity's Transform is; everything else is an added component.
// phase is an optional editor-side grouping label such as "mid boss".
// The runtime ignores it; it only organizes the editor's lists and track.
struct EnemyDefinition {
	std::string id;
	std::string phase;
	glm::vec2 size{ 40.0f, 40.0f };
	// a list of { "type": "sprite", ... } objects, in the order they are shown
	nlohmann::json components = nlohmann::json::array();
};

// A single spawn event: when, what, and where.
// It carries no phase of its own - that belongs to the enemy it names.
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

// An editor-side grouping label. The colour is what ties it together on the track.
struct PhaseDefinition {
	std::string name;
	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct StageDefinition {
	PlayerDefinition player;
	// declared separately so a phase can exist before anything is put in it
	std::vector<PhaseDefinition> phases;
	std::vector<EnemyDefinition> enemies;
	std::vector<TimelineEntry> timeline;
};

// Both throw nlohmann::json::exception on malformed input
StageDefinition LoadStageDefinition(const std::string& utf8Path);
void SaveStageDefinition(const StageDefinition& stage, const std::string& utf8Path);

// null when the timeline names an enemy id that no longer exists
const EnemyDefinition* FindEnemy(const StageDefinition& stage, const std::string& id);

// A spawn shows the phase of whatever it spawns, so tagging an enemy moves all of them at once.
const std::string& EntryPhase(const StageDefinition& stage, const TimelineEntry& entry);

// Components are a short list, so a scan keeps every caller simple. Null when absent.
nlohmann::json* FindComponent(nlohmann::json& components, const std::string& type);
const nlohmann::json* FindComponent(const nlohmann::json& components, const std::string& type);

// The movement component's pattern, which is what the waypoint helpers below work on.
nlohmann::json* FindMovementPattern(EnemyDefinition& enemy);

// Waypoints live inside the movement JSON, so their shape is edited in one place.
// Inserting puts the new point halfway to the next one, or past the end when it is the last.
// An index of -1 inserts before the first point, which is how an empty path gets started.
void InsertWaypointAfter(nlohmann::json& movement, int index);
void RemoveWaypoint(nlohmann::json& movement, int index);
int WaypointCount(const nlohmann::json& movement);

const PhaseDefinition* FindPhase(const StageDefinition& stage, const std::string& name);

// A distinct starting colour per name, so a new phase is never invisible before one is picked.
glm::vec4 DefaultPhaseColor(const std::string& name);

// The phase's own colour, or the default when the name belongs to no phase any more.
glm::vec4 PhaseColorOf(const StageDefinition& stage, const std::string& name);

}
