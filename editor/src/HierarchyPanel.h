#pragma once

#include "StageDefinition.h"

#include <string>

namespace Editor {

// Lists the stage's player (pinned), its phases, and the enemy roster grouped under them
class HierarchyPanel {
private:
	bool playerSelected = false;
	int selectedIndex = -1;
	int selectedPhase = -1;
	std::string newPhaseName;

	void AddPhase(StageDefinition& stage);

public:
	// returns true if the selection changed this frame
	bool Draw(const char* title, StageDefinition& stage);

	bool IsPlayerSelected() const { return playerSelected; }
	int GetSelected() const { return selectedIndex; }
	int GetSelectedPhase() const { return selectedPhase; }
	void SelectPlayer() { playerSelected = true; selectedIndex = -1; selectedPhase = -1; }
	void ClearSelection() { playerSelected = false; selectedIndex = -1; selectedPhase = -1; }
};

}
