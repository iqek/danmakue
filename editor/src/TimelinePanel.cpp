#include "TimelinePanel.h"

#include <algorithm>
#include <cfloat>
#include <imgui.h>
#include <string>

namespace Editor {

bool TimelinePanel::Draw(const char* title, StageDefinition& stage){
	bool changed = false;

	ImGui::Begin(title);

	// structural changes are deferred to the end of the frame, so they can't
	// resize the vector the row loops are walking
	int removeIndex = -1;
	int duplicateIndex = -1;
	// -2 means no request, -1 means unphased, otherwise an index into phases
	int newEntryPhase = -2;

	if(ImGui::Button("New Timeline Entry")){
		newEntryPhase = -1;
	}

	bool entrySelected = selectedIndex >= 0 && selectedIndex < static_cast<int>(stage.timeline.size());
	ImGui::BeginDisabled(!entrySelected);
	if(ImGui::Button("Duplicate")){
		duplicateIndex = selectedIndex;
	}
	ImGui::EndDisabled();

	ImGui::Separator();

	// filters the scene overlay's markers only - the list below always
	// shows everything, or there'd be entries you couldn't select
	ImGui::TextDisabled("Show in scene");
	const char* modeNames[] = { "All", "Selected only", "By enemy", "By phase" };
	int modeIndex = static_cast<int>(spawnVisibility.mode);
	ImGui::SetNextItemWidth(-FLT_MIN);
	if(ImGui::Combo("##showInScene", &modeIndex, modeNames, IM_ARRAYSIZE(modeNames))){
		spawnVisibility.mode = static_cast<SpawnVisibility::Mode>(modeIndex);
		if(spawnVisibility.mode == SpawnVisibility::Mode::ByEnemy && spawnVisibility.enemyId.empty() && !stage.enemies.empty()){
			spawnVisibility.enemyId = stage.enemies[0].id;
		}
		if(spawnVisibility.mode == SpawnVisibility::Mode::ByPhase && spawnVisibility.phase.empty() && !stage.phases.empty()){
			spawnVisibility.phase = stage.phases[0];
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

	if(spawnVisibility.mode == SpawnVisibility::Mode::ByPhase){
		ImGui::SetNextItemWidth(-FLT_MIN);
		if(ImGui::BeginCombo("##showPhase", spawnVisibility.phase.empty() ? "(none)" : spawnVisibility.phase.c_str())){
			if(ImGui::Selectable("(none)", spawnVisibility.phase.empty())){
				spawnVisibility.phase.clear();
			}
			for(const auto& phase : stage.phases){
				if(ImGui::Selectable(phase.c_str(), phase == spawnVisibility.phase)){
					spawnVisibility.phase = phase;
				}
			}
			ImGui::EndCombo();
		}
	}

	ImGui::Separator();

	float removeButtonWidth = ImGui::GetFrameHeight();

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

	auto drawEntryRow = [&](int i){
		ImGui::PushID(i);

		std::string label = std::to_string(i) + ": " + stage.timeline[i].spawnId;
		float selectableWidth = ImGui::GetContentRegionAvail().x - removeButtonWidth - ImGui::GetStyle().ItemSpacing.x;
		if(ImGui::Selectable(label.c_str(), selectedIndex == i, 0, ImVec2(selectableWidth, 0.0f))){
			selectedIndex = i;
			changed = true;
		}

		// distinct payload type from the Hierarchy's, so an enemy can't be
		// dropped here and have its index address the wrong vector
		if(ImGui::BeginDragDropSource()){
			ImGui::SetDragDropPayload("TIMELINE_ROW", &i, sizeof(int));
			ImGui::TextUnformatted(label.c_str());
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
			drawPhaseMenuItems(stage.timeline[i].phase);
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if(ImGui::SmallButton("X")){
			removeIndex = i;
		}

		ImGui::PopID();
	};

	auto acceptEntryDrop = [&](const std::string& phase){
		if(ImGui::BeginDragDropTarget()){
			if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TIMELINE_ROW")){
				int dragged = *static_cast<const int*>(payload->Data);
				if(dragged >= 0 && dragged < static_cast<int>(stage.timeline.size())){
					stage.timeline[dragged].phase = phase;
				}
			}
			ImGui::EndDragDropTarget();
		}
	};

	// an entry tagged with a phase that no longer exists still has to appear
	// somewhere, so it falls back into the unphased group rather than vanishing
	auto isUnphased = [&](const TimelineEntry& entry){
		return entry.phase.empty() || std::find(stage.phases.begin(), stage.phases.end(), entry.phase) == stage.phases.end();
	};

	auto drawUnphased = [&](){
		for(int i = 0; i < static_cast<int>(stage.timeline.size()); i++){
			if(isUnphased(stage.timeline[i])){
				drawEntryRow(i);
			}
		}
	};

	for(int p = 0; p < static_cast<int>(stage.phases.size()); p++){
		std::string phase = stage.phases[p];

		ImGui::PushID(phase.c_str());
		bool open = ImGui::TreeNodeEx(phase.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanTextWidth);
		acceptEntryDrop(phase);

		if(ImGui::BeginPopupContextItem()){
			if(ImGui::MenuItem("New Entry Here")){
				newEntryPhase = p;
			}
			ImGui::EndPopup();
		}

		if(open){
			for(int i = 0; i < static_cast<int>(stage.timeline.size()); i++){
				if(stage.timeline[i].phase == phase){
					drawEntryRow(i);
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
		acceptEntryDrop(std::string());

		if(ImGui::BeginPopupContextItem()){
			if(ImGui::MenuItem("New Entry Here")){
				newEntryPhase = -1;
			}
			ImGui::EndPopup();
		}

		if(open){
			drawUnphased();
			ImGui::TreePop();
		}
	}

	// only fires over empty space, so it doesn't fight the per-row menus
	if(ImGui::BeginPopupContextWindow("timelineContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)){
		if(ImGui::MenuItem("New Timeline Entry")){
			newEntryPhase = -1;
		}
		ImGui::EndPopup();
	}

	if(newEntryPhase != -2){
		TimelineEntry entry;
		// a valid default trigger keeps this entry loadable even before
		// its trigger/spawn target is actually set up
		entry.trigger = nlohmann::json{ { "type", "time" }, { "seconds", 1.0 } };
		entry.spawnId = stage.enemies.empty() ? std::string() : stage.enemies[0].id;
		if(newEntryPhase >= 0 && newEntryPhase < static_cast<int>(stage.phases.size())){
			entry.phase = stage.phases[newEntryPhase];
		}
		stage.timeline.push_back(entry);
		selectedIndex = static_cast<int>(stage.timeline.size()) - 1;
		changed = true;
	}

	if(duplicateIndex >= 0 && duplicateIndex < static_cast<int>(stage.timeline.size())){
		TimelineEntry copy = stage.timeline[duplicateIndex];
		// nudge it, or the copy's marker sits exactly under the original's
		// in the scene view and neither can be grabbed apart from the other
		copy.position.x += 30.0f;
		copy.position.y += 30.0f;
		// right after the source, since trigger order is the stage's running order
		stage.timeline.insert(stage.timeline.begin() + duplicateIndex + 1, copy);
		selectedIndex = duplicateIndex + 1;
		changed = true;
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
