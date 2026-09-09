#pragma once

#include "StageDefinition.h"

namespace Editor {

// Lists the stage's player (pinned) and enemy roster, tracks the selection
class HierarchyPanel {
private:
	bool playerSelected = false;
	int selectedIndex = -1;

public:
	// returns true if the selection changed this frame
	bool Draw(const char* title, StageDefinition& stage);

	bool IsPlayerSelected() const { return playerSelected; }
	int GetSelected() const { return selectedIndex; }
	void SelectPlayer() { playerSelected = true; selectedIndex = -1; }
	void ClearSelection() { playerSelected = false; selectedIndex = -1; }
};

}
