#pragma once

namespace Editor {

// What the panels currently have picked, so the Inspector and the scene overlay agree.
// At most one of these is set at a time; the rest stay at their empty value.
struct Selection {
	bool player = false;
	int enemy = -1;
	int phase = -1;
	int timeline = -1;
};

}
