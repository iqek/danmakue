#pragma once

#include <string>

namespace Editor {

// Lets any one docked panel temporarily fill the whole editor window,
// toggled by double-clicking inside it - like Unity's tab-maximize, though
// this triggers off a panel's content rather than hooking its tab label
// specifically (that needs unstable ImGui internals beyond what this uses)
class PanelMaximizer {
private:
	std::string maximizedTitle;

public:
	// call once per frame, before drawing any panels
	void Update();

	// false means don't call this panel's Draw() at all this frame - it's
	// hidden because a different panel is currently maximized
	bool ShouldDraw(const char* title) const;

	// call before this panel's first Begin() call this frame
	void PrepareNextWindow(const char* title) const;
};

}
