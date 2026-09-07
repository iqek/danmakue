// glad must be included before any header that pulls in GLFW/gl.h
#include <glad/glad.h>

#include "engine/core/Log.h"
#include "engine/platform/Window.h"
#include "AssetBrowserPanel.h"
#include "DefaultLayout.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "LogPanel.h"
#include "LogSink.h"
#include "PlayController.h"
#include "StageDefinition.h"
#include "StagePanel.h"
#include "TimelinePanel.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>

int main(){
	Engine::Log::Init();
	ENGINE_INFO("Editor started");

	Editor::LogPanel logPanel;
	auto logSink = std::make_shared<Editor::LogSink>(logPanel);
	Engine::Log::GetCoreLogger()->sinks().push_back(logSink);
	Engine::Log::GetClientLogger()->sinks().push_back(logSink);

	Editor::AssetBrowserPanel assetBrowser(ASSETS_DIR);
	Editor::HierarchyPanel hierarchyPanel;
	Editor::TimelinePanel timelinePanel;
	Editor::InspectorPanel inspectorPanel;
	Editor::PlayController playController(SANDBOX_EXECUTABLE);

	Editor::StagePanel stagePanel(std::string(ASSETS_DIR) + "/stages", "stage1.json");
	Editor::StageDefinition stage;

	auto loadCurrentStage = [&](){
		try{
			stage = Editor::LoadStageDefinition(stagePanel.GetCurrentPath());
			ENGINE_INFO("Loaded stage: {}", stagePanel.GetCurrentPath());
		}
		catch(const std::exception& e){
			ENGINE_CORE_ERROR("Failed to load stage {}: {}", stagePanel.GetCurrentPath(), e.what());
		}
	};
	loadCurrentStage();

	Engine::Window window({ "Danmakue Editor", 1600, 900 });

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 410");

	bool dockLayoutInitialized = false;

	while(!window.ShouldClose()){
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if(!dockLayoutInitialized){
			Editor::SetupDefaultDockLayoutIfNeeded();
			dockLayoutInitialized = true;
		}

		ImGui::DockSpaceOverViewport(Editor::GetMainDockspaceId(), ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

		logPanel.Draw("Log");
		assetBrowser.Draw("Assets");

		bool enemySelectionChanged = hierarchyPanel.Draw("Hierarchy", stage);
		bool timelineSelectionChanged = timelinePanel.Draw("Timeline", stage);
		if(enemySelectionChanged){
			timelinePanel.ClearSelection();
		}
		if(timelineSelectionChanged){
			hierarchyPanel.ClearSelection();
		}

		inspectorPanel.Draw("Inspector", stage, hierarchyPanel.IsPlayerSelected(), hierarchyPanel.GetSelected(), timelinePanel.GetSelected());
		playController.Draw("Play", stagePanel.GetCurrentFileName());

		if(stagePanel.Draw("Stage", stage)){
			hierarchyPanel.ClearSelection();
			timelinePanel.ClearSelection();
			loadCurrentStage();
		}

		ImGui::Render();

		glViewport(0, 0, window.GetWidth(), window.GetHeight());
		glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.OnUpdate();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
