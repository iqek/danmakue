#include "SceneOverlay.h"
#include "JsonFieldHelpers.h"

#include <string>
#include <vector>

namespace Editor {

namespace {

constexpr float handleRadius = 7.0f;

const ImU32 spawnColor = IM_COL32(255, 190, 80, 255);
const ImU32 dimSpawnColor = IM_COL32(255, 190, 80, 130);
const ImU32 selectedColor = IM_COL32(255, 255, 255, 255);
const ImU32 playerColor = IM_COL32(255, 140, 165, 255);

ImVec2 WorldToScreen(const glm::vec2& world, const ImVec2& imageMin, float scale){
	return ImVec2(imageMin.x + world.x * scale, imageMin.y + world.y * scale);
}

int FindEnemyIndex(const StageDefinition& stage, const std::string& id){
	for(int i = 0; i < static_cast<int>(stage.enemies.size()); i++){
		if(stage.enemies[i].id == id){
			return i;
		}
	}
	return -1;
}

// a path wears its enemy's phase colour, so several drawn at once still read apart
ImU32 PathColor(const StageDefinition& stage, const EnemyDefinition& enemy, bool emphasised){
	float alpha = emphasised ? 1.0f : 0.45f;
	if(enemy.phase.empty()){
		return IM_COL32(120, 230, 220, static_cast<int>(alpha * 255.0f));
	}
	glm::vec4 color = PhaseColorOf(stage, enemy.phase);
	return ImGui::GetColorU32(ImVec4(color.r, color.g, color.b, alpha));
}

// moves world by the mouse delta while held; clicked reports the press itself
bool DragHandle(const char* id, glm::vec2& world, const ImVec2& imageMin, float scale, bool& clicked){
	ImVec2 screen = WorldToScreen(world, imageMin, scale);

	ImGui::SetCursorScreenPos(ImVec2(screen.x - handleRadius, screen.y - handleRadius));
	ImGui::InvisibleButton(id, ImVec2(handleRadius * 2.0f, handleRadius * 2.0f));

	clicked = ImGui::IsItemActivated();

	if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
		ImVec2 delta = ImGui::GetIO().MouseDelta;
		world.x += delta.x / scale;
		world.y += delta.y / scale;
		return true;
	}

	return false;
}

void DrawMarker(ImDrawList* drawList, const ImVec2& screen, ImU32 color, bool selected, const std::string& label){
	drawList->AddCircle(screen, handleRadius, color, 0, selected ? 2.5f : 1.5f);
	drawList->AddCircleFilled(screen, 2.0f, color);
	drawList->AddText(ImVec2(screen.x + handleRadius + 3.0f, screen.y - 7.0f), color, label.c_str());
}

// anchor is the spawn point the path hangs off, since waypoints are stored as offsets
void DrawWaypoints(EnemyDefinition& enemy, const glm::vec2& anchor, ImU32 color, const ImVec2& imageMin, float scale, ImDrawList* drawList){
	nlohmann::json* pattern = FindMovementPattern(enemy);
	if(pattern == nullptr || pattern->value("type", std::string()) != "waypoint"){
		return;
	}
	if(!pattern->contains("waypoints") || !pattern->at("waypoints").is_array()){
		return;
	}

	nlohmann::json& waypoints = (*pattern)["waypoints"];

	ImGui::PushID("waypoints");

	// deferred, so the menu can't resize the array this loop is walking
	int addAfter = -2;
	int removeAt = -1;

	// path first, so the handles sit on top of the lines
	glm::vec2 previous = anchor;
	for(int i = 0; i < static_cast<int>(waypoints.size()); i++){
		glm::vec2 point = anchor + GetVec2(waypoints[i], "offset", glm::vec2(0.0f));
		drawList->AddLine(WorldToScreen(previous, imageMin, scale), WorldToScreen(point, imageMin, scale), color, 1.5f);
		previous = point;
	}

	for(int i = 0; i < static_cast<int>(waypoints.size()); i++){
		ImGui::PushID(i);

		glm::vec2 point = anchor + GetVec2(waypoints[i], "offset", glm::vec2(0.0f));
		bool clicked = false;
		if(DragHandle("##waypoint", point, imageMin, scale, clicked)){
			SetVec2(waypoints[i], "offset", point - anchor);
		}

		if(ImGui::BeginPopupContextItem()){
			if(ImGui::MenuItem("Add Point After")){
				addAfter = i;
			}
			if(ImGui::MenuItem("Delete Point")){
				removeAt = i;
			}
			ImGui::EndPopup();
		}

		DrawMarker(drawList, WorldToScreen(point, imageMin, scale), color, false, std::to_string(i));

		ImGui::PopID();
	}

	ImGui::PopID();

	if(addAfter != -2){
		InsertWaypointAfter(*pattern, addAfter);
	}
	if(removeAt >= 0){
		RemoveWaypoint(*pattern, removeAt);
	}
}

}

bool SelectionCovers(const StageDefinition& stage, const Selection& selection, int timelineIndex){
	const TimelineEntry& entry = stage.timeline[timelineIndex];

	if(selection.timeline >= 0){
		return timelineIndex == selection.timeline;
	}
	if(selection.enemy >= 0 && selection.enemy < static_cast<int>(stage.enemies.size())){
		return entry.spawnId == stage.enemies[selection.enemy].id;
	}
	if(selection.phase >= 0 && selection.phase < static_cast<int>(stage.phases.size())){
		return EntryPhase(stage, entry) == stage.phases[selection.phase].name;
	}
	return false;
}

SceneOverlayResult DrawSceneOverlay(StageDefinition& stage, const ImVec2& imageMin, float scale, const Selection& selection, const SceneViewOptions& options){
	SceneOverlayResult result;
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// paths first so the spawn markers stay clickable on top of their own handles
	ImGui::PushID("paths");
	for(int i = 0; i < static_cast<int>(stage.timeline.size()); i++){
		bool covered = SelectionCovers(stage, selection, i);
		if(!options.showAllPaths && !covered){
			continue;
		}

		int enemyIndex = FindEnemyIndex(stage, stage.timeline[i].spawnId);
		if(enemyIndex < 0){
			continue;
		}

		// scoped per spawn, or one template drawn at several anchors reuses its handle ids
		ImGui::PushID(i);
		DrawWaypoints(stage.enemies[enemyIndex], stage.timeline[i].position, PathColor(stage, stage.enemies[enemyIndex], covered), imageMin, scale, drawList);
		ImGui::PopID();
	}
	ImGui::PopID();

	ImGui::PushID("timeline");
	for(int i = 0; i < static_cast<int>(stage.timeline.size()); i++){
		bool covered = SelectionCovers(stage, selection, i);
		if(!options.showAllSpawns && !covered){
			continue;
		}

		ImGui::PushID(i);

		bool clicked = false;
		DragHandle("##spawn", stage.timeline[i].position, imageMin, scale, clicked);
		if(clicked){
			result.clickedTimelineIndex = i;
		}

		if(ImGui::BeginPopupContextItem()){
			int enemyIndex = FindEnemyIndex(stage, stage.timeline[i].spawnId);
			nlohmann::json* pattern = enemyIndex >= 0 ? FindMovementPattern(stage.enemies[enemyIndex]) : nullptr;
			bool followsPath = pattern != nullptr && pattern->value("type", std::string()) == "waypoint";
			// the way into an empty path, where there is no point to right click yet
			if(ImGui::MenuItem("Add Waypoint", nullptr, false, followsPath)){
				InsertWaypointAfter(*pattern, WaypointCount(*pattern) - 1);
			}
			ImGui::EndPopup();
		}

		bool selected = i == selection.timeline;
		ImU32 color = selected ? selectedColor : (covered ? spawnColor : dimSpawnColor);
		DrawMarker(drawList, WorldToScreen(stage.timeline[i].position, imageMin, scale), color, selected, std::to_string(i) + ": " + stage.timeline[i].spawnId);

		ImGui::PopID();
	}
	ImGui::PopID();

	ImGui::PushID("player");
	bool playerClicked = false;
	DragHandle("##playerSpawn", stage.player.position, imageMin, scale, playerClicked);
	result.clickedPlayer = playerClicked;
	DrawMarker(drawList, WorldToScreen(stage.player.position, imageMin, scale), selection.player ? selectedColor : playerColor, selection.player, "Player");
	ImGui::PopID();

	return result;
}

}
