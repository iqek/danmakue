#include "TimelinePanel.h"

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
