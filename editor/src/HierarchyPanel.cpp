#include "HierarchyPanel.h"

#include <imgui.h>
#include <string>

namespace Editor {

namespace {

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

bool HierarchyPanel::Draw(const char* title, StageDefinition& stage){
	bool changed = false;

	ImGui::Begin(title);

	if(ImGui::Selectable("Player", playerSelected)){
		playerSelected = true;
		selectedIndex = -1;
		changed = true;
	}

	ImGui::Separator();

	if(ImGui::Button("New Enemy")){
		EnemyDefinition enemy;
		enemy.id = GenerateUniqueEnemyId(stage);
		// a zero-velocity default keeps the movement JSON valid so the enemy
		// can actually spawn even before its movement is set up
		enemy.movement = nlohmann::json{ { "type", "linear" }, { "velocity", nlohmann::json::array({ 0.0, 0.0 }) } };
		stage.enemies.push_back(enemy);
		playerSelected = false;
		selectedIndex = static_cast<int>(stage.enemies.size()) - 1;
		changed = true;
	}

	ImGui::Separator();

	int removeIndex = -1;
	float removeButtonWidth = ImGui::GetFrameHeight();

	for(int i = 0; i < static_cast<int>(stage.enemies.size()); i++){
		ImGui::PushID(i);

		float selectableWidth = ImGui::GetContentRegionAvail().x - removeButtonWidth - ImGui::GetStyle().ItemSpacing.x;
		if(ImGui::Selectable(stage.enemies[i].id.c_str(), selectedIndex == i, 0, ImVec2(selectableWidth, 0.0f))){
			playerSelected = false;
			selectedIndex = i;
			changed = true;
		}

		ImGui::SameLine();
		if(ImGui::SmallButton("X")){
			removeIndex = i;
		}

		ImGui::PopID();
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
