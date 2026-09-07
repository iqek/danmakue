#pragma once

#include "StageDefinition.h"

namespace Editor {

// Edits whichever player/enemy/timeline entry is currently selected
class InspectorPanel {
public:
	void Draw(const char* title, StageDefinition& stage, bool playerSelected, int selectedEnemyIndex, int selectedTimelineIndex);
};

}
