#include "PanelMaximizer.h"

// GImGui->HoveredWindow isn't part of the public API, but it's the only way
// to learn which named window was double-clicked without also intercepting
// clicks meant for that window's own widgets
#include <imgui_internal.h>

namespace Editor {

void PanelMaximizer::Update(){
	if(!ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)){
		return;
	}

	ImGuiWindow* hovered = GImGui->HoveredWindow;
	if(hovered == nullptr){
		return;
	}

	std::string name = hovered->Name;
	maximizedTitle = (maximizedTitle == name) ? std::string() : name;
}

bool PanelMaximizer::ShouldDraw(const char* title) const{
	return maximizedTitle.empty() || maximizedTitle == title;
}

void PanelMaximizer::PrepareNextWindow(const char* title) const{
	if(maximizedTitle != title){
		return;
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowFocus();
}

}
