#pragma once

#include "StageDefinition.h"

namespace Editor {

// Lists a stage's spawn timeline and tracks which entry is selected.
// A plain list for now; a natural place to grow into a visual timeline strip.
class TimelinePanel {
private:
	int selectedIndex = -1;

public:
	// returns true if the selection changed this frame
	bool Draw(const char* title, StageDefinition& stage);

	int GetSelected() const { return selectedIndex; }
	void ClearSelection() { selectedIndex = -1; }
};

}
