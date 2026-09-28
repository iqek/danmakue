#include "HierarchyPanel.h"
#include "ComponentTypes.h"

#include "engine/core/Log.h"

#include <algorithm>
#include <cctype>
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
		if(!IsIdTaken(stage, candidate)){
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

	if(FindPhase(stage, newPhaseName) != nullptr){
		ENGINE_CORE_ERROR("Phase already exists: {}", newPhaseName);
		return;
	}

	PhaseDefinition phase;
	phase.name = newPhaseName;
	phase.color = DefaultPhaseColor(phase.name);
	stage.phases.push_back(phase);
	newPhaseName.clear();
}

bool HierarchyPanel::Draw(const char* title, StageDefinition& stage){
	bool changed = false;

	ImGui::Begin(title);

	if(ImGui::Selectable("Player", playerSelected)){
		playerSelected = true;
		selectedIndex = -1;
		selectedPhase = -1;
		changed = true;
	}

	ImGui::Separator();

	// every structural change is deferred, so it can't resize the vectors being walked
	int removeIndex = -1;
	int removePhaseIndex = -1;
	int duplicateIndex = -1;
	// -2 means no request, -1 means unphased, otherwise an index into phases
	int newEnemyPhase = -2;

	auto drawPhaseMenuItems = [&](std::string& phase){
		if(ImGui::BeginMenu("Move to phase")){
			if(ImGui::MenuItem("(none)", nullptr, phase.empty())){
				phase.clear();
			}
			for(const auto& definition : stage.phases){
				if(ImGui::MenuItem(definition.name.c_str(), nullptr, definition.name == phase)){
					phase = definition.name;
				}
			}
			ImGui::EndMenu();
		}
	};

	auto drawEnemyRow = [&](int i){
		ImGui::PushID(i);

		if(ImGui::Selectable(stage.enemies[i].id.c_str(), selectedIndex == i)){
			playerSelected = false;
			selectedPhase = -1;
			selectedIndex = i;
			changed = true;
		}

		// a distinct payload type keeps these off the Timeline's rows, which index another vector
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

		ImGui::PopID();
	};

	// tagging the enemy is what moves every spawn of it, so this panel owns phases
	auto acceptPhaseDrop = [&](const std::string& phase){
		if(!ImGui::BeginDragDropTarget()){
			return;
		}
		if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENEMY_ROW")){
			int dragged = *static_cast<const int*>(payload->Data);
			if(dragged >= 0 && dragged < static_cast<int>(stage.enemies.size())){
				stage.enemies[dragged].phase = phase;
			}
		}
		ImGui::EndDragDropTarget();
	};

	// an enemy tagged with a phase that no longer exists falls back here instead of vanishing
	auto isUnphased = [&](const EnemyDefinition& enemy){
		return enemy.phase.empty() || FindPhase(stage, enemy.phase) == nullptr;
	};

	auto drawUnphased = [&](){
		for(int i = 0; i < static_cast<int>(stage.enemies.size()); i++){
			if(isUnphased(stage.enemies[i])){
				drawEnemyRow(i);
			}
		}
	};

	for(int p = 0; p < static_cast<int>(stage.phases.size()); p++){
		std::string phase = stage.phases[p].name;
		const glm::vec4& color = stage.phases[p].color;

		ImGui::PushID(phase.c_str());

		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanTextWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if(selectedPhase == p){
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
		}

		// the header wears the phase's own colour, so it matches the track at a glance
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, 1.0f));
		bool open = ImGui::TreeNodeEx(phase.c_str(), nodeFlags);
		ImGui::PopStyleColor();

		if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()){
			playerSelected = false;
			selectedIndex = -1;
			selectedPhase = p;
			changed = true;
		}

		acceptPhaseDrop(phase);

		if(ImGui::BeginPopupContextItem()){
			if(ImGui::MenuItem("New Enemy Here")){
				newEnemyPhase = p;
			}
			if(ImGui::MenuItem("Delete Phase")){
				removePhaseIndex = p;
			}
			ImGui::EndPopup();
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

	bool anyUnphased = false;
	for(const auto& enemy : stage.enemies){
		if(isUnphased(enemy)){
			anyUnphased = true;
			break;
		}
	}

	// the group is clutter when it is empty, but it still has to exist as a drop target
	const ImGuiPayload* carried = ImGui::GetDragDropPayload();
	bool draggingEnemy = carried != nullptr && carried->IsDataType("ENEMY_ROW");

	if(stage.phases.empty()){
		drawUnphased();
	}
	else if(anyUnphased || draggingEnemy){
		bool open = ImGui::TreeNodeEx("(unphased)", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanTextWidth);
		acceptPhaseDrop(std::string());

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
		enemy.components = DefaultEnemyComponents();
		if(newEnemyPhase >= 0 && newEnemyPhase < static_cast<int>(stage.phases.size())){
			enemy.phase = stage.phases[newEnemyPhase].name;
		}
		stage.enemies.push_back(enemy);
		playerSelected = false;
		selectedPhase = -1;
		selectedIndex = static_cast<int>(stage.enemies.size()) - 1;
		changed = true;
	}

	if(duplicateIndex >= 0 && duplicateIndex < static_cast<int>(stage.enemies.size())){
		EnemyDefinition copy = stage.enemies[duplicateIndex];
		copy.id = GenerateCopyId(stage, copy.id);
		// insert beside the source, since there is still no way to reorder this list
		stage.enemies.insert(stage.enemies.begin() + duplicateIndex + 1, copy);
		selectedIndex = duplicateIndex + 1;
		playerSelected = false;
		selectedPhase = -1;
		changed = true;
	}

	if(removePhaseIndex >= 0){
		// deleting a phase only unlabels its members, it never deletes content
		std::string removed = stage.phases[removePhaseIndex].name;
		stage.phases.erase(stage.phases.begin() + removePhaseIndex);
		for(auto& enemy : stage.enemies){
			if(enemy.phase == removed){
				enemy.phase.clear();
			}
		}
		if(selectedPhase == removePhaseIndex){
			selectedPhase = -1;
		}
		else if(selectedPhase > removePhaseIndex){
			selectedPhase--;
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
