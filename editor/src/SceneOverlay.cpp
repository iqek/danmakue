#include "SceneOverlay.h"
#include "JsonFieldHelpers.h"

#include <string>

namespace Editor {

namespace {

constexpr float handleRadius = 7.0f;

const ImU32 spawnColor = IM_COL32(255, 190, 80, 255);
const ImU32 selectedColor = IM_COL32(255, 255, 255, 255);
const ImU32 playerColor = IM_COL32(255, 140, 165, 255);
const ImU32 waypointColor = IM_COL32(120, 230, 220, 255);

ImVec2 WorldToScreen(const glm::vec2& world, const ImVec2& imageMin, float scale){
	return ImVec2(imageMin.x + world.x * scale, imageMin.y + world.y * scale);
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

void DrawWaypoints(EnemyDefinition& enemy, const ImVec2& imageMin, float scale, ImDrawList* drawList){
	if(enemy.movement.value("type", std::string()) != "waypoint"){
		return;
	}
	if(!enemy.movement.contains("waypoints") || !enemy.movement.at("waypoints").is_array()){
		return;
	}

	nlohmann::json& waypoints = enemy.movement["waypoints"];

	ImGui::PushID("waypoints");

	// path first, so the handles sit on top of the lines
	for(int i = 0; i + 1 < static_cast<int>(waypoints.size()); i++){
		glm::vec2 from = GetVec2(waypoints[i], "position", glm::vec2(0.0f));
		glm::vec2 to = GetVec2(waypoints[i + 1], "position", glm::vec2(0.0f));
		drawList->AddLine(WorldToScreen(from, imageMin, scale), WorldToScreen(to, imageMin, scale), waypointColor, 1.5f);
	}

	for(int i = 0; i < static_cast<int>(waypoints.size()); i++){
		ImGui::PushID(i);

		glm::vec2 point = GetVec2(waypoints[i], "position", glm::vec2(0.0f));
		bool clicked = false;
		if(DragHandle("##waypoint", point, imageMin, scale, clicked)){
			SetVec2(waypoints[i], "position", point);
		}

		DrawMarker(drawList, WorldToScreen(point, imageMin, scale), waypointColor, false, std::to_string(i));

		ImGui::PopID();
	}

	ImGui::PopID();
}

}

bool SpawnVisibility::IsVisible(const TimelineEntry& entry, int index, int selectedIndex) const{
	switch(mode){
		case Mode::SelectedOnly:
			return index == selectedIndex;
		case Mode::ByEnemy:
			return entry.spawnId == enemyId;
		case Mode::ByPhase:
			return entry.phase == phase;
		default:
			return true;
	}
}

SceneOverlayResult DrawSceneOverlay(StageDefinition& stage, const ImVec2& imageMin, float scale, const SpawnVisibility& visibility, bool playerSelected, int selectedEnemyIndex, int selectedTimelineIndex){
	SceneOverlayResult result;
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// show the path of the selected enemy, or of whatever the selected
	// timeline entry spawns - so picking a spawn point also shows where it goes
	int waypointEnemyIndex = selectedEnemyIndex;
	if(waypointEnemyIndex < 0 && selectedTimelineIndex >= 0 && selectedTimelineIndex < static_cast<int>(stage.timeline.size())){
		const std::string& spawnId = stage.timeline[selectedTimelineIndex].spawnId;
		for(int i = 0; i < static_cast<int>(stage.enemies.size()); i++){
			if(stage.enemies[i].id == spawnId){
				waypointEnemyIndex = i;
				break;
			}
		}
	}

	if(waypointEnemyIndex >= 0 && waypointEnemyIndex < static_cast<int>(stage.enemies.size())){
		DrawWaypoints(stage.enemies[waypointEnemyIndex], imageMin, scale, drawList);
	}

	ImGui::PushID("timeline");
	for(int i = 0; i < static_cast<int>(stage.timeline.size()); i++){
		TimelineEntry& entry = stage.timeline[i];
		if(!visibility.IsVisible(entry, i, selectedTimelineIndex)){
			continue;
		}

		ImGui::PushID(i);

		bool clicked = false;
		DragHandle("##spawn", entry.position, imageMin, scale, clicked);
		if(clicked){
			result.clickedTimelineIndex = i;
		}

		bool selected = (i == selectedTimelineIndex);
		DrawMarker(drawList, WorldToScreen(entry.position, imageMin, scale), selected ? selectedColor : spawnColor, selected, std::to_string(i) + ": " + entry.spawnId);

		ImGui::PopID();
	}
	ImGui::PopID();

	ImGui::PushID("player");
	bool playerClicked = false;
	DragHandle("##playerSpawn", stage.player.position, imageMin, scale, playerClicked);
	result.clickedPlayer = playerClicked;
	DrawMarker(drawList, WorldToScreen(stage.player.position, imageMin, scale), playerSelected ? selectedColor : playerColor, playerSelected, "Player");
	ImGui::PopID();

	return result;
}

}
