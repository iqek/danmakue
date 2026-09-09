#include "HierarchyPanel.h"

#include "engine/core/Log.h"

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <string>

namespace Editor {

namespace {

bool IsIdTaken(const StageDefinition& stage, const std::string& candidate){
	for(const auto& enemy : stage.enemies){
		if(enemy.id == candidate){
			return true;
		}
	}
	return false;
}

// "bat" -> "bat2", and a copy of "bat2" -> "bat3" rather than "bat22"
std::string GenerateCopyId(const StageDefinition& stage, const std::string& sourceId){
	std::string base = sourceId;
	while(!base.empty() && std::isdigit(static_cast<unsigned char>(base.back()))){
		base.pop_back();
	}
	if(base.empty()){
		base = "enemy";
	}

	for(int suffix = 2; ; suffix++){
		std::string candidate = base + std::to_string(suffix);
		if(!IsIdTaken(stage, candidate)){
			return candidate;
		}
	}
}

std::string GenerateUniqueEnemyId(const StageDefinition& stage){
	int index = 0;
	while(true){
		std::string candidate = "enemy" + std::to_string(index);

		bool collision = false;
		for(const auto& enemy : stage.enemies){
			if(enemy.id == candidate){
				collision = true;
				break;
			}
		}

		if(!collision){
			return candidate;
		}
		index++;
	}
}

}

void HierarchyPanel::AddPhase(StageDefinition& stage){
	if(newPhaseName.empty()){
		return;
	}

	if(std::find(stage.phases.begin(), stage.phases.end(), newPhaseName) != stage.phases.end()){
		ENGINE_CORE_ERROR("Phase already exists: {}", newPhaseName);
		return;
	}

	stage.phases.push_back(newPhaseName);
	newPhaseName.clear();
}

bool HierarchyPanel::Draw(const char* title, StageDefinition& stage){
	bool changed = false;

	ImGui::Begin(title);

	if(ImGui::Selectable("Player", playerSelected)){
		playerSelected = true;
		selectedIndex = -1;
		changed = true;
	}

	ImGui::Separator();

	// every structural change is deferred to the end of the frame - doing it
	// inline would resize the vectors the row loops are walking
	int removeIndex = -1;
	int removePhaseIndex = -1;
	int duplicateIndex = -1;
	// -2 means no request, -1 means unphased, otherwise an index into phases
	int newEnemyPhase = -2;
	float removeButtonWidth = ImGui::GetFrameHeight();

	if(ImGui::Button("New Enemy")){
		newEnemyPhase = -1;
	}

	bool enemySelected = selectedIndex >= 0 && selectedIndex < static_cast<int>(stage.enemies.size());
	ImGui::BeginDisabled(!enemySelected);
	if(ImGui::Button("Duplicate")){
		duplicateIndex = selectedIndex;
	}
	ImGui::EndDisabled();

	ImGui::Separator();

	ImGui::SetNextItemWidth(-FLT_MIN);
	if(ImGui::InputTextWithHint("##newPhase", "new phase name", &newPhaseName, ImGuiInputTextFlags_EnterReturnsTrue)){
		AddPhase(stage);
	}
	if(ImGui::Button("Add Phase")){
		AddPhase(stage);
	}

	ImGui::Separator();

	auto drawPhaseMenuItems = [&](std::string& phase){
		if(ImGui::BeginMenu("Move to phase")){
			if(ImGui::MenuItem("(none)", nullptr, phase.empty())){
				phase.clear();
			}
			for(const auto& name : stage.phases){
				if(ImGui::MenuItem(name.c_str(), nullptr, name == phase)){
					phase = name;
				}
			}
			ImGui::EndMenu();
		}
	};

	auto drawEnemyRow = [&](int i){
		ImGui::PushID(i);

		float selectableWidth = ImGui::GetContentRegionAvail().x - removeButtonWidth - ImGui::GetStyle().ItemSpacing.x;
		if(ImGui::Selectable(stage.enemies[i].id.c_str(), selectedIndex == i, 0, ImVec2(selectableWidth, 0.0f))){
			playerSelected = false;
			selectedIndex = i;
			changed = true;
		}

		// the payload type keeps these from being dropped on the Timeline's
		// phase headers, where the index would address the wrong vector
		if(ImGui::BeginDragDropSource()){
			ImGui::SetDragDropPayload("ENEMY_ROW", &i, sizeof(int));
			ImGui::TextUnformatted(stage.enemies[i].id.c_str());
			ImGui::EndDragDropSource();
		}

		if(ImGui::BeginPopupContextItem()){
			if(ImGui::MenuItem("Duplicate")){
				duplicateIndex = i;
			}
			if(ImGui::MenuItem("Delete")){
				removeIndex = i;
			}
			ImGui::Separator();
			drawPhaseMenuItems(stage.enemies[i].phase);
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if(ImGui::SmallButton("X")){
			removeIndex = i;
		}

		ImGui::PopID();
	};

	auto acceptEnemyDrop = [&](const std::string& phase){
		if(ImGui::BeginDragDropTarget()){
			if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENEMY_ROW")){
				int dragged = *static_cast<const int*>(payload->Data);
				if(dragged >= 0 && dragged < static_cast<int>(stage.enemies.size())){
					stage.enemies[dragged].phase = phase;
				}
			}
			ImGui::EndDragDropTarget();
		}
	};

	// an enemy tagged with a phase that no longer exists still has to appear
	// somewhere, so it falls back into the unphased group rather than vanishing
	auto isUnphased = [&](const EnemyDefinition& enemy){
		return enemy.phase.empty() || std::find(stage.phases.begin(), stage.phases.end(), enemy.phase) == stage.phases.end();
	};

	auto drawUnphased = [&](){
		for(int i = 0; i < static_cast<int>(stage.enemies.size()); i++){
			if(isUnphased(stage.enemies[i])){
				drawEnemyRow(i);
			}
		}
	};

	for(int p = 0; p < static_cast<int>(stage.phases.size()); p++){
		std::string phase = stage.phases[p];

		ImGui::PushID(phase.c_str());
		bool open = ImGui::TreeNodeEx(phase.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanTextWidth);
		acceptEnemyDrop(phase);

		if(ImGui::BeginPopupContextItem()){
			if(ImGui::MenuItem("New Enemy Here")){
				newEnemyPhase = p;
			}
			if(ImGui::MenuItem("Delete Phase")){
				removePhaseIndex = p;
			}
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if(ImGui::SmallButton("X")){
			removePhaseIndex = p;
		}

		if(open){
			for(int i = 0; i < static_cast<int>(stage.enemies.size()); i++){
				if(stage.enemies[i].phase == phase){
					drawEnemyRow(i);
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	if(stage.phases.empty()){
		drawUnphased();
	}
	else{
		bool open = ImGui::TreeNodeEx("(unphased)", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanTextWidth);
		acceptEnemyDrop(std::string());

		if(ImGui::BeginPopupContextItem()){
			if(ImGui::MenuItem("New Enemy Here")){
				newEnemyPhase = -1;
			}
			ImGui::EndPopup();
		}

		if(open){
			drawUnphased();
			ImGui::TreePop();
		}
	}

	// only fires over empty space, so it doesn't fight the per-row menus
	if(ImGui::BeginPopupContextWindow("hierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)){
		if(ImGui::MenuItem("New Enemy")){
			newEnemyPhase = -1;
		}
		ImGui::Separator();
		ImGui::SetNextItemWidth(160.0f);
		if(ImGui::InputTextWithHint("##contextPhase", "new phase name", &newPhaseName, ImGuiInputTextFlags_EnterReturnsTrue)){
			AddPhase(stage);
			ImGui::CloseCurrentPopup();
		}
		if(ImGui::MenuItem("Add Phase")){
			AddPhase(stage);
		}
		ImGui::EndPopup();
	}

	if(newEnemyPhase != -2){
		EnemyDefinition enemy;
		enemy.id = GenerateUniqueEnemyId(stage);
		// a zero-velocity default keeps the movement JSON valid so the enemy
		// can actually spawn even before its movement is set up
		enemy.movement = nlohmann::json{ { "type", "linear" }, { "velocity", nlohmann::json::array({ 0.0, 0.0 }) } };
		if(newEnemyPhase >= 0 && newEnemyPhase < static_cast<int>(stage.phases.size())){
			enemy.phase = stage.phases[newEnemyPhase];
		}
		stage.enemies.push_back(enemy);
		playerSelected = false;
		selectedIndex = static_cast<int>(stage.enemies.size()) - 1;
		changed = true;
	}

	if(duplicateIndex >= 0 && duplicateIndex < static_cast<int>(stage.enemies.size())){
		EnemyDefinition copy = stage.enemies[duplicateIndex];
		copy.id = GenerateCopyId(stage, copy.id);
		// insert next to the source rather than at the end - there's no way
		// to reorder the list yet, so appending would strand it
		stage.enemies.insert(stage.enemies.begin() + duplicateIndex + 1, copy);
		selectedIndex = duplicateIndex + 1;
		playerSelected = false;
		changed = true;
	}

	if(removePhaseIndex >= 0){
		// deleting a phase only unlabels its members, it never deletes content
		std::string removed = stage.phases[removePhaseIndex];
		stage.phases.erase(stage.phases.begin() + removePhaseIndex);
		for(auto& enemy : stage.enemies){
			if(enemy.phase == removed){
				enemy.phase.clear();
			}
		}
		for(auto& entry : stage.timeline){
			if(entry.phase == removed){
				entry.phase.clear();
			}
		}
	}

	if(removeIndex >= 0){
		stage.enemies.erase(stage.enemies.begin() + removeIndex);
		if(selectedIndex == removeIndex){
			selectedIndex = -1;
		}
		else if(selectedIndex > removeIndex){
			selectedIndex--;
		}
	}

	ImGui::End();

	return changed;
}

}
