#include "InspectorPanel.h"
#include "EmitterListEditor.h"
#include "JsonFieldHelpers.h"

#include "engine/core/Log.h"

#include <algorithm>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Editor {

namespace {

using Json = nlohmann::json;

void DrawMovementEditor(Json& movement){
	std::string type = movement.value("type", std::string("linear"));

	const char* typeNames[] = { "linear", "waypoint" };
	int typeIndex = (type == "waypoint") ? 1 : 0;

	if(ImGui::Combo("Type##movement", &typeIndex, typeNames, IM_ARRAYSIZE(typeNames))){
		std::string newType = typeNames[typeIndex];
		if(newType == "linear"){
			movement = Json{ { "type", "linear" }, { "velocity", Json::array({ 0.0, 0.0 }) } };
		}
		else{
			movement = Json{ { "type", "waypoint" }, { "speed", 100.0 }, { "waypoints", Json::array() } };
		}
		return;
	}

	if(type == "linear"){
		glm::vec2 velocity = GetVec2(movement, "velocity", glm::vec2(0.0f));
		if(ImGui::DragFloat2("Velocity", &velocity.x)){
			SetVec2(movement, "velocity", velocity);
		}
	}
	else if(type == "waypoint"){
		float speed = movement.value("speed", 100.0f);
		if(ImGui::DragFloat("Speed", &speed, 1.0f, 0.0f, 2000.0f)){
			SetFloat(movement, "speed", speed);
		}

		ImGui::TextDisabled("Offsets are from the spawn point, so the path travels with it");

		if(!movement.contains("waypoints") || !movement.at("waypoints").is_array()){
			movement["waypoints"] = Json::array();
		}
		Json& waypoints = movement["waypoints"];

		int removeIndex = -1;
		for(int i = 0; i < static_cast<int>(waypoints.size()); i++){
			ImGui::PushID(i);
			Json& point = waypoints[i];

			glm::vec2 offset = GetVec2(point, "offset", glm::vec2(0.0f));
			if(ImGui::DragFloat2("Offset", &offset.x)){
				SetVec2(point, "offset", offset);
			}

			float waitTime = point.value("waitTime", 0.0f);
			if(ImGui::DragFloat("Wait Time", &waitTime, 0.1f, 0.0f, 60.0f)){
				SetFloat(point, "waitTime", waitTime);
			}

			if(ImGui::Button("Remove Waypoint")){
				removeIndex = i;
			}

			ImGui::Separator();
			ImGui::PopID();
		}

		if(removeIndex >= 0){
			RemoveWaypoint(movement, removeIndex);
		}

		if(ImGui::Button("Add Waypoint")){
			InsertWaypointAfter(movement, WaypointCount(movement) - 1);
		}
	}
}

void DrawTriggerEditor(Json& trigger){
	std::string type = trigger.value("type", std::string("delay"));

	const char* typeNames[] = { "delay", "afterCleared" };
	int typeIndex = (type == "afterCleared") ? 1 : 0;

	if(ImGui::Combo("Type##trigger", &typeIndex, typeNames, IM_ARRAYSIZE(typeNames))){
		std::string newType = typeNames[typeIndex];
		if(newType == "delay"){
			trigger = Json{ { "type", "delay" }, { "seconds", 1.0 } };
		}
		else{
			trigger = Json{ { "type", "afterCleared" } };
		}
		return;
	}

	if(type == "afterCleared"){
		ImGui::TextDisabled("Waits until nothing is left alive");
		return;
	}

	float seconds = trigger.value("seconds", 1.0f);
	if(ImGui::DragFloat("Wait (s)", &seconds, 0.1f, 0.0f, 3600.0f)){
		SetFloat(trigger, "seconds", std::max(0.0f, seconds));
	}
	ImGui::TextDisabled("Counted from the previous entry; zero spawns with it");
}

void DrawPhaseCombo(std::string& phase, const StageDefinition& stage){
	if(ImGui::BeginCombo("Phase", phase.empty() ? "(none)" : phase.c_str())){
		if(ImGui::Selectable("(none)", phase.empty())){
			phase.clear();
		}
		for(const auto& definition : stage.phases){
			if(ImGui::Selectable(definition.name.c_str(), definition.name == phase)){
				phase = definition.name;
			}
		}
		ImGui::EndCombo();
	}
}

void DrawSpawnIdCombo(TimelineEntry& entry, const StageDefinition& stage){
	if(ImGui::BeginCombo("Spawn Id", entry.spawnId.c_str())){
		for(const auto& enemy : stage.enemies){
			if(ImGui::Selectable(enemy.id.c_str(), enemy.id == entry.spawnId)){
				entry.spawnId = enemy.id;
			}
		}
		ImGui::EndCombo();
	}
}

void DrawEnemyInspector(EnemyDefinition& enemy, const StageDefinition& stage){
	ImGui::InputText("Id", &enemy.id);
	DrawPhaseCombo(enemy.phase, stage);
	ImGui::DragFloat2("Size", &enemy.size.x);
	ImGui::DragFloat2("Collider Size", &enemy.colliderSize.x);
	ImGui::ColorEdit4("Color", &enemy.color.x);
	ImGui::DragInt("Health", &enemy.health, 1.0f, 1, 10000);

	ImGui::Spacing();
	ImGui::SeparatorText("Movement");
	DrawMovementEditor(enemy.movement);

	ImGui::Spacing();
	ImGui::SeparatorText("Emitters");
	DrawEmitterList(enemy.emitters);
}

void DrawTimelineInspector(TimelineEntry& entry, const StageDefinition& stage){
	DrawSpawnIdCombo(entry, stage);

	// read-only here: the phase is the enemy's, so every spawn of it moves together
	const std::string& phase = EntryPhase(stage, entry);
	ImGui::Text("Phase: %s", phase.empty() ? "(none)" : phase.c_str());
	ImGui::TextDisabled("Set it on the enemy in the Hierarchy");

	ImGui::DragFloat2("Position", &entry.position.x);

	ImGui::Spacing();
	ImGui::SeparatorText("Trigger");
	DrawTriggerEditor(entry.trigger);
}

void DrawPlayerInspector(PlayerDefinition& player){
	ImGui::DragFloat2("Start Position", &player.position.x);
	ImGui::DragFloat2("Size", &player.size.x);
	ImGui::DragFloat2("Collider Size", &player.colliderSize.x);
	ImGui::ColorEdit4("Color", &player.color.x);
	ImGui::DragInt("Lives", &player.lives, 1.0f, 1, 99);
	ImGui::DragFloat("Move Speed", &player.moveSpeed, 1.0f, 0.0f, 2000.0f);

	ImGui::Spacing();
	ImGui::SeparatorText("Weapons");
	DrawEmitterList(player.weapons);
}

}

void InspectorPanel::DrawPhaseInspector(StageDefinition& stage, int index){
	PhaseDefinition& phase = stage.phases[index];

	if(renamingPhase != index){
		renamingPhase = index;
		renameBuffer = phase.name;
	}

	ImGui::InputText("Name", &renameBuffer);
	if(ImGui::IsItemDeactivatedAfterEdit() && !renameBuffer.empty() && renameBuffer != phase.name){
		if(FindPhase(stage, renameBuffer) != nullptr){
			ENGINE_CORE_ERROR("Phase already exists: {}", renameBuffer);
			renameBuffer = phase.name;
		}
		else{
			// every enemy tagged with the old name has to follow the rename
			for(auto& enemy : stage.enemies){
				if(enemy.phase == phase.name){
					enemy.phase = renameBuffer;
				}
			}
			phase.name = renameBuffer;
		}
	}

	ImGui::ColorEdit4("Color", &phase.color.x, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaPreviewHalf);
	ImGui::TextDisabled("Used by the Hierarchy and the Timeline track");

	int used = 0;
	for(const auto& enemy : stage.enemies){
		if(enemy.phase == phase.name){
			used++;
		}
	}

	ImGui::Spacing();
	ImGui::Text("%d enemy template(s) in this phase", used);
}

void InspectorPanel::Draw(const char* title, StageDefinition& stage, const Selection& selection){
	ImGui::Begin(title);

	if(selection.player){
		DrawPlayerInspector(stage.player);
	}
	else if(selection.enemy >= 0 && selection.enemy < static_cast<int>(stage.enemies.size())){
		DrawEnemyInspector(stage.enemies[selection.enemy], stage);
	}
	else if(selection.phase >= 0 && selection.phase < static_cast<int>(stage.phases.size())){
		DrawPhaseInspector(stage, selection.phase);
	}
	else if(selection.timeline >= 0 && selection.timeline < static_cast<int>(stage.timeline.size())){
		DrawTimelineInspector(stage.timeline[selection.timeline], stage);
	}
	else{
		ImGui::TextDisabled("Nothing selected");
	}

	ImGui::End();
}

}
