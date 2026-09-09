#include "InspectorPanel.h"
#include "EmitterListEditor.h"
#include "JsonFieldHelpers.h"

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

		if(!movement.contains("waypoints") || !movement.at("waypoints").is_array()){
			movement["waypoints"] = Json::array();
		}
		Json& waypoints = movement["waypoints"];

		int removeIndex = -1;
		for(int i = 0; i < static_cast<int>(waypoints.size()); i++){
			ImGui::PushID(i);
			Json& point = waypoints[i];

			glm::vec2 position = GetVec2(point, "position", glm::vec2(0.0f));
			if(ImGui::DragFloat2("Position", &position.x)){
				SetVec2(point, "position", position);
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
			waypoints.erase(waypoints.begin() + removeIndex);
		}

		if(ImGui::Button("Add Waypoint")){
			Json point;
			SetVec2(point, "position", glm::vec2(0.0f));
			point["waitTime"] = 1.0;
			waypoints.push_back(point);
		}
	}
}

void DrawTriggerEditor(Json& trigger){
	std::string type = trigger.value("type", std::string("time"));

	const char* typeNames[] = { "time", "afterCleared" };
	int typeIndex = (type == "afterCleared") ? 1 : 0;

	if(ImGui::Combo("Type##trigger", &typeIndex, typeNames, IM_ARRAYSIZE(typeNames))){
		std::string newType = typeNames[typeIndex];
		if(newType == "time"){
			trigger = Json{ { "type", "time" }, { "seconds", 1.0 } };
		}
		else{
			trigger = Json{ { "type", "afterCleared" } };
		}
		return;
	}

	if(type == "time"){
		float seconds = trigger.value("seconds", 1.0f);
		if(ImGui::DragFloat("Seconds", &seconds, 0.1f, 0.0f, 3600.0f)){
			SetFloat(trigger, "seconds", seconds);
		}
	}
	else{
		ImGui::TextDisabled("Fires once every enemy is gone");
	}
}

void DrawPhaseCombo(std::string& phase, const StageDefinition& stage){
	if(ImGui::BeginCombo("Phase", phase.empty() ? "(none)" : phase.c_str())){
		if(ImGui::Selectable("(none)", phase.empty())){
			phase.clear();
		}
		for(const auto& name : stage.phases){
			if(ImGui::Selectable(name.c_str(), name == phase)){
				phase = name;
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
	DrawPhaseCombo(entry.phase, stage);
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

void InspectorPanel::Draw(const char* title, StageDefinition& stage, bool playerSelected, int selectedEnemyIndex, int selectedTimelineIndex){
	ImGui::Begin(title);

	if(playerSelected){
		DrawPlayerInspector(stage.player);
	}
	else if(selectedEnemyIndex >= 0 && selectedEnemyIndex < static_cast<int>(stage.enemies.size())){
		DrawEnemyInspector(stage.enemies[selectedEnemyIndex], stage);
	}
	else if(selectedTimelineIndex >= 0 && selectedTimelineIndex < static_cast<int>(stage.timeline.size())){
		DrawTimelineInspector(stage.timeline[selectedTimelineIndex], stage);
	}
	else{
		ImGui::TextDisabled("Nothing selected");
	}

	ImGui::End();
}

}
