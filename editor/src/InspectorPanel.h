#pragma once

#include "Selection.h"
#include "StageDefinition.h"

#include <string>

namespace Editor {

// Edits whichever player, enemy, phase or timeline entry is currently selected
class InspectorPanel {
private:
	// a rename only lands when the field is left, so enemies aren't retagged per keystroke
	int renamingPhase = -1;
	std::string renameBuffer;

	void DrawPhaseInspector(StageDefinition& stage, int index);

public:
	void Draw(const char* title, StageDefinition& stage, const Selection& selection);
};

}
