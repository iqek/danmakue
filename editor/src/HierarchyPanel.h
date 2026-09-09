#pragma once

#include "StageDefinition.h"

#include <string>

namespace Editor {

// Lists the stage's player (pinned) and enemy roster grouped by phase,
// and tracks the selection
class HierarchyPanel {
private:
	bool playerSelected = false;
	int selectedIndex = -1;
	std::string newPhaseName;

	void AddPhase(StageDefinition& stage);

public:
	// returns true if the selection changed this frame
	bool Draw(const char* title, StageDefinition& stage);

	bool IsPlayerSelected() const { return playerSelected; }
	int GetSelected() const { return selectedIndex; }
	void SelectPlayer() { playerSelected = true; selectedIndex = -1; }
	void ClearSelection() { playerSelected = false; selectedIndex = -1; }
};

}
