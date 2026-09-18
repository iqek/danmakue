#pragma once

#include "Selection.h"
#include "StageDefinition.h"

#include <imgui.h>

namespace Editor {

// extra game units shown around the play area while stopped - spawn points
// deliberately sit offscreen, so they'd be unreachable without this. kept
// tight so the play area itself stays large; a marker dragged past it can
// still be recovered from the Inspector's position field
constexpr float kSceneEditMargin = 120.0f;

// What the overlay draws beyond whatever is selected.
// With both off it follows the selection: a spawn, an enemy's spawns, or a whole phase.
struct SceneViewOptions {
	bool showAllSpawns = true;
	bool showAllPaths = false;
};

struct SceneOverlayResult {
	int clickedTimelineIndex = -1;
	bool clickedPlayer = false;
};

// Whether the selection is about this spawn: itself, its enemy, or its enemy's phase.
// Both SceneViewOptions toggles fall back to this when they are off.
bool SelectionCovers(const StageDefinition& stage, const Selection& selection, int timelineIndex);

// Draggable markers over the game view for the positions that are actually stored
// fields: each timeline entry's spawn point, the player's spawn, and the waypoint
// paths hanging off those spawns. Everything else on screen at runtime is computed
// from movement, so there's nothing to drag for it.
// imageMin is where world (0,0) sits on screen, scale converts game units to pixels.
SceneOverlayResult DrawSceneOverlay(StageDefinition& stage, const ImVec2& imageMin, float scale, const Selection& selection, const SceneViewOptions& options);

}
