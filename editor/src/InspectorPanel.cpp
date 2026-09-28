#include "InspectorPanel.h"
#include "ComponentTypes.h"
#include "EmitterListEditor.h"
#include "JsonFieldHelpers.h"

#include "engine/core/Log.h"

#include <algorithm>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Editor {

namespace {

using Json = nlohmann::json;

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
	ImGui::TextDisabled("Size is always here; everything else is a component below");

	ImGui::Spacing();

	int removeIndex = -1;
	for(int i = 0; i < static_cast<int>(enemy.components.size()); i++){
		Json& component = enemy.components[i];
		std::string type = component.value("type", std::string());
		const ComponentType* kind = FindComponentType(type);

		ImGui::PushID(i);

		// the header's close button is the remove, the way Unity does it
		bool keep = true;
		bool open = ImGui::CollapsingHeader(kind != nullptr ? kind->label : type.c_str(), &keep, ImGuiTreeNodeFlags_DefaultOpen);
		if(!keep){
			removeIndex = i;
		}

		if(open){
			ImGui::Indent();
			if(kind != nullptr){
				kind->draw(component);
			}
			else{
				ImGui::TextDisabled("This build does not know this component");
			}
			ImGui::Unindent();
		}

		ImGui::PopID();
	}

	if(removeIndex >= 0){
		enemy.components.erase(enemy.components.begin() + removeIndex);
	}

	ImGui::Spacing();
	if(ImGui::Button("Add Component")){
		ImGui::OpenPopup("addComponent");
	}

	if(ImGui::BeginPopup("addComponent")){
		bool anyLeft = false;
		for(const auto& kind : ComponentTypes()){
			if(FindComponent(enemy.components, kind.type) != nullptr){
				continue;
			}
			anyLeft = true;
			if(ImGui::MenuItem(kind.label)){
				enemy.components.push_back(kind.makeDefault());
			}
		}
		if(!anyLeft){
			ImGui::TextDisabled("Nothing left to add");
		}
		ImGui::EndPopup();
	}
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
