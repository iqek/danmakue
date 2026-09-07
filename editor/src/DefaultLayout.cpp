#include "DefaultLayout.h"

// DockBuilder is only declared here, not in imgui.h - it's meant for exactly
// this use case (baking a default layout) but the API is flagged unstable
#include <imgui_internal.h>

namespace Editor {

ImGuiID GetMainDockspaceId(){
	return ImHashStr("MainDockSpace");
}

void SetupDefaultDockLayoutIfNeeded(){
	ImGuiID dockspaceId = GetMainDockspaceId();
	if(ImGui::DockBuilderGetNode(dockspaceId) != nullptr){
		return;
	}

	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

	ImGuiID mainId = dockspaceId;
	ImGuiID leftId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Left, 0.078f, nullptr, &mainId);
	ImGuiID leftBottomId = ImGui::DockBuilderSplitNode(leftId, ImGuiDir_Down, 0.29f, nullptr, &leftId);
	ImGuiID rightId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Right, 0.118f, nullptr, &mainId);
	ImGuiID bottomId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Down, 0.192f, nullptr, &mainId);
	ImGuiID playId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Up, 0.034f, nullptr, &mainId);
	// mainId is left over as the big central area, currently held by Stage -
	// this is where the embedded Play viewport will eventually live instead

	ImGui::DockBuilderDockWindow("Hierarchy", leftId);
	ImGui::DockBuilderDockWindow("Timeline", leftBottomId);
	ImGui::DockBuilderDockWindow("Inspector", rightId);
	ImGui::DockBuilderDockWindow("Play", playId);
	ImGui::DockBuilderDockWindow("Stage", mainId);
	ImGui::DockBuilderDockWindow("Assets", bottomId);
	ImGui::DockBuilderDockWindow("Log", bottomId);

	// Play and Stage are single-control utility panels - a tab label just
	// wastes vertical space, so their tab bar starts out hidden
	if(ImGuiDockNode* playNode = ImGui::DockBuilderGetNode(playId)){
		playNode->SetLocalFlags(ImGuiDockNodeFlags_HiddenTabBar);
	}
	if(ImGuiDockNode* stageNode = ImGui::DockBuilderGetNode(mainId)){
		stageNode->SetLocalFlags(ImGuiDockNodeFlags_HiddenTabBar);
	}

	ImGui::DockBuilderFinish(dockspaceId);
}

}
