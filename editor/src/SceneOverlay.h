#pragma once

#include "StageDefinition.h"

#include <imgui.h>

namespace Editor {

// extra game units shown around the play area while stopped - spawn points
// deliberately sit offscreen, so they'd be unreachable without this. kept
// tight so the play area itself stays large; a marker dragged past it can
// still be recovered from the Inspector's position field
constexpr float kSceneEditMargin = 120.0f;

// Which spawn markers the scene overlay draws - a full stage has far more
// spawn points than are useful to look at all at once
struct SpawnVisibility {
	enum class Mode {
		All,
		SelectedOnly,
		ByEnemy,
		ByPhase,
	};

	Mode mode = Mode::All;
	std::string enemyId;
	std::string phase;

	bool IsVisible(const TimelineEntry& entry, int index, int selectedIndex) const;
};

struct SceneOverlayResult {
	int clickedTimelineIndex = -1;
	bool clickedPlayer = false;
};

// Draggable markers over the game view for the positions that are actually
// stored fields: each timeline entry's spawn point, the player's spawn, and
// the waypoints of whichever enemy is in context. Everything else on screen
// at runtime is computed from movement, so there's nothing to drag for it.
// imageMin is where world (0,0) sits on screen, scale converts game units to pixels
SceneOverlayResult DrawSceneOverlay(StageDefinition& stage, const ImVec2& imageMin, float scale, const SpawnVisibility& visibility, bool playerSelected, int selectedEnemyIndex, int selectedTimelineIndex);

}
