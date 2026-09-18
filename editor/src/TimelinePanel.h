#pragma once

#include "SceneOverlay.h"
#include "StageDefinition.h"

#include <vector>

namespace Editor {

// Where one timeline entry sits once the delays ahead of it are walked.
struct TimelineSlot {
	// seconds from the start of its own segment, not from the stage start
	float time = 0.0f;
	// clear gates split the stage into segments, since their length is unknowable
	int segment = 0;
	// row within a stack of entries that all spawn in the same instant
	int lane = 0;
	bool isGate = false;
	// waits zero seconds, so it spawns together with the entry before it
	bool sharesInstant = false;
	// a clear gate with nothing spawned yet, which is already satisfied when reached
	bool emptyGate = false;
};

// Walks the timeline's delays into the positions above.
std::vector<TimelineSlot> ComputeTimelineLayout(const StageDefinition& stage);

// Moves one entry to a new instant within its own segment, reordering it if it passed a neighbour.
// Every other entry keeps the instant it had, and selectedIndex follows the moved entry.
// slots must be the layout of the stage as it is now, before any of this frame's edits.
void RetimeEntry(StageDefinition& stage, const std::vector<TimelineSlot>& slots, int index, float time, int& selectedIndex);

// Shows the spawn timeline as a time track plus an ordered list of its entries.
class TimelinePanel {
private:
	int selectedIndex = -1;
	float pixelsPerSecond = 60.0f;
	SceneViewOptions viewOptions;
	// a drag is only committed on release, so the order can't shift under ImGui's active id
	int draggingIndex = -1;
	float dragTime = 0.0f;

public:
	// returns true if the selection changed this frame
	bool Draw(const char* title, StageDefinition& stage);

	const SceneViewOptions& GetSceneViewOptions() const { return viewOptions; }

	int GetSelected() const { return selectedIndex; }
	void SetSelected(int index) { selectedIndex = index; }
	void ClearSelection() { selectedIndex = -1; }
};

}
