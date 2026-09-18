#pragma once

#include <imgui.h>

namespace Editor {

// Stable id for the main dockspace, so it can be named before the dockspace exists.
ImGuiID GetMainDockspaceId();

// Builds the default panel layout, but only when imgui.ini restored none.
// Once the user rearranges panels that file takes over and this is skipped.
// Call it after a NewFrame() and before this frame's DockSpaceOverViewport().
void SetupDefaultDockLayoutIfNeeded();

// Throws away the current layout and rebuilds the default one.
// Same frame ordering rules as SetupDefaultDockLayoutIfNeeded().
void ResetDockLayout();

}
