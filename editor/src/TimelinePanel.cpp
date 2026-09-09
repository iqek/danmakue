#include "TimelinePanel.h"

#include <cfloat>
#include <imgui.h>
#include <string>

namespace Editor {

bool TimelinePanel::Draw(const char* title, StageDefinition& stage){
	bool changed = false;

	ImGui::Begin(title);

	if(ImGui::Button("New Timeline Entry")){
		TimelineEntry entry;
		// a valid default trigger keeps this entry loadable even before
		// its trigger/spawn target is actually set up
		entry.trigger = nlohmann::json{ { "type", "time" }, { "seconds", 1.0 } };
		entry.spawnId = stage.enemies.empty() ? std::string() : stage.enemies[0].id;
		stage.timeline.push_back(entry);
		selectedIndex = static_cast<int>(stage.timeline.size()) - 1;
		changed = true;
	}

	bool entrySelected = selectedIndex >= 0 && selectedIndex < static_cast<int>(stage.timeline.size());
	ImGui::BeginDisabled(!entrySelected);
	if(ImGui::Button("Duplicate")){
		TimelineEntry copy = stage.timeline[selectedIndex];
		// nudge it, or the copy's marker sits exactly under the original's
		// in the scene view and neither can be grabbed apart from the other
		copy.position.x += 30.0f;
		copy.position.y += 30.0f;
		// right after the source, since trigger order is the stage's running order
		stage.timeline.insert(stage.timeline.begin() + selectedIndex + 1, copy);
		selectedIndex++;
		changed = true;
	}
	ImGui::EndDisabled();

	ImGui::Separator();

	// filters the scene overlay's markers only - the list below always
	// shows everything, or there'd be entries you couldn't select
	ImGui::TextDisabled("Show in scene");
	const char* modeNames[] = { "All", "Selected only", "By enemy" };
	int modeIndex = static_cast<int>(spawnVisibility.mode);
	ImGui::SetNextItemWidth(-FLT_MIN);
	if(ImGui::Combo("##showInScene", &modeIndex, modeNames, IM_ARRAYSIZE(modeNames))){
		spawnVisibility.mode = static_cast<SpawnVisibility::Mode>(modeIndex);
		if(spawnVisibility.mode == SpawnVisibility::Mode::ByEnemy && spawnVisibility.enemyId.empty() && !stage.enemies.empty()){
			spawnVisibility.enemyId = stage.enemies[0].id;
		}
	}

	if(spawnVisibility.mode == SpawnVisibility::Mode::ByEnemy){
		ImGui::SetNextItemWidth(-FLT_MIN);
		if(ImGui::BeginCombo("##showEnemy", spawnVisibility.enemyId.c_str())){
			for(const auto& enemy : stage.enemies){
				if(ImGui::Selectable(enemy.id.c_str(), enemy.id == spawnVisibility.enemyId)){
					spawnVisibility.enemyId = enemy.id;
				}
			}
			ImGui::EndCombo();
		}
	}

	ImGui::Separator();

	int removeIndex = -1;
	float removeButtonWidth = ImGui::GetFrameHeight();

	for(int i = 0; i < static_cast<int>(stage.timeline.size()); i++){
		ImGui::PushID(i);

		std::string label = std::to_string(i) + ": " + stage.timeline[i].spawnId;
		float selectableWidth = ImGui::GetContentRegionAvail().x - removeButtonWidth - ImGui::GetStyle().ItemSpacing.x;
		if(ImGui::Selectable(label.c_str(), selectedIndex == i, 0, ImVec2(selectableWidth, 0.0f))){
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
		stage.timeline.erase(stage.timeline.begin() + removeIndex);
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
