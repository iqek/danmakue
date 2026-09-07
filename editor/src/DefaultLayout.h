#pragma once

#include <imgui.h>

namespace Editor {

// Stable id for the main dockspace, independent of ImGui's own per-viewport
// auto-generated id (which is awkward to compute before the dockspace exists)
ImGuiID GetMainDockspaceId();

// Builds the default panel layout for that id, but only if no layout was
// already restored from imgui.ini - once the user rearranges panels, that
// file takes over and this is skipped on every later launch. Must be called
// after at least one ImGui::NewFrame() (for a valid viewport size) and
// before this frame's DockSpaceOverViewport call.
void SetupDefaultDockLayoutIfNeeded();

}
