#include "DefaultLayout.h"

// DockBuilder lives here rather than imgui.h: it is meant for baking layouts, but flagged unstable
#include <imgui_internal.h>

namespace Editor {

namespace {

void BuildDefaultLayout(ImGuiID dockspaceId){
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

	ImGuiID mainId = dockspaceId;
	ImGuiID leftId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Left, 0.16f, nullptr, &mainId);
	ImGuiID rightId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Right, 0.2f, nullptr, &mainId);
	ImGuiID bottomId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Down, 0.24f, nullptr, &mainId);
	// the timeline is a wide time axis, so it gets the full width under the stage view
	ImGuiID timelineId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Down, 0.36f, nullptr, &mainId);
	ImGuiID playId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Up, 0.06f, nullptr, &mainId);

	ImGui::DockBuilderDockWindow("Hierarchy", leftId);
	ImGui::DockBuilderDockWindow("Inspector", rightId);
	ImGui::DockBuilderDockWindow("Timeline", timelineId);
	ImGui::DockBuilderDockWindow("Play", playId);
	ImGui::DockBuilderDockWindow("Stage", mainId);
	ImGui::DockBuilderDockWindow("Assets", bottomId);
	ImGui::DockBuilderDockWindow("Log", bottomId);

	// Play and Stage hold one control each, so a tab label would only cost height
	if(ImGuiDockNode* playNode = ImGui::DockBuilderGetNode(playId)){
		playNode->SetLocalFlags(ImGuiDockNodeFlags_HiddenTabBar);
	}
	if(ImGuiDockNode* stageNode = ImGui::DockBuilderGetNode(mainId)){
		stageNode->SetLocalFlags(ImGuiDockNodeFlags_HiddenTabBar);
	}

	ImGui::DockBuilderFinish(dockspaceId);
}

}

ImGuiID GetMainDockspaceId(){
	return ImHashStr("MainDockSpace");
}

void SetupDefaultDockLayoutIfNeeded(){
	ImGuiID dockspaceId = GetMainDockspaceId();
	if(ImGui::DockBuilderGetNode(dockspaceId) != nullptr){
		return;
	}

	BuildDefaultLayout(dockspaceId);
}

void ResetDockLayout(){
	ImGuiID dockspaceId = GetMainDockspaceId();
	ImGui::DockBuilderRemoveNode(dockspaceId);
	BuildDefaultLayout(dockspaceId);
}

}
