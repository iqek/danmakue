// glad must be included before any header that pulls in GLFW/gl.h
#include <glad/glad.h>

#include "engine/core/Log.h"
#include "engine/core/Time.h"
#include "engine/platform/Input.h"
#include "engine/platform/Window.h"
#include "AssetBrowserPanel.h"
#include "DefaultLayout.h"
#include "EmbeddedGame.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "LogPanel.h"
#include "LogSink.h"
#include "PanelMaximizer.h"
#include "StageDefinition.h"
#include "StagePanel.h"
#include "TimelinePanel.h"

#include <algorithm>
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
	Editor::PanelMaximizer maximizer;

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
	Engine::Input::Init(window.GetHandle());

	Editor::EmbeddedGame embeddedGame(std::string(ASSETS_DIR) + "/PressStart2P-Regular.ttf");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 410");

	bool dockLayoutInitialized = false;

	while(!window.ShouldClose()){
		Engine::Time::Update();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if(!dockLayoutInitialized){
			Editor::SetupDefaultDockLayoutIfNeeded();
			dockLayoutInitialized = true;
		}

		maximizer.Update();

		ImGui::DockSpaceOverViewport(Editor::GetMainDockspaceId(), ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

		if(maximizer.ShouldDraw("Log")){
			maximizer.PrepareNextWindow("Log");
			logPanel.Draw("Log");
		}

		if(maximizer.ShouldDraw("Assets")){
			maximizer.PrepareNextWindow("Assets");
			assetBrowser.Draw("Assets");
		}

		bool enemySelectionChanged = false;
		if(maximizer.ShouldDraw("Hierarchy")){
			maximizer.PrepareNextWindow("Hierarchy");
			enemySelectionChanged = hierarchyPanel.Draw("Hierarchy", stage);
		}

		bool timelineSelectionChanged = false;
		if(maximizer.ShouldDraw("Timeline")){
			maximizer.PrepareNextWindow("Timeline");
			timelineSelectionChanged = timelinePanel.Draw("Timeline", stage);
		}

		if(enemySelectionChanged){
			timelinePanel.ClearSelection();
		}
		if(timelineSelectionChanged){
			hierarchyPanel.ClearSelection();
		}

		if(maximizer.ShouldDraw("Inspector")){
			maximizer.PrepareNextWindow("Inspector");
			inspectorPanel.Draw("Inspector", stage, hierarchyPanel.IsPlayerSelected(), hierarchyPanel.GetSelected(), timelinePanel.GetSelected());
		}

		if(maximizer.ShouldDraw("Play")){
			maximizer.PrepareNextWindow("Play");
			ImGui::Begin("Play");
			if(embeddedGame.IsPlaying()){
				if(ImGui::Button("Stop")){
					embeddedGame.Stop();
				}
				ImGui::SameLine();
				ImGui::TextDisabled("Running...");
			}
			else{
				if(ImGui::Button("Play")){
					embeddedGame.Start(stage, stagePanel.GetCurrentPath());
				}
			}
			ImGui::End();
		}

		bool inputAllowed = false;
		if(maximizer.ShouldDraw("Stage")){
			maximizer.PrepareNextWindow("Stage");

			if(stagePanel.Draw("Stage", stage)){
				hierarchyPanel.ClearSelection();
				timelinePanel.ClearSelection();
				loadCurrentStage();
				embeddedGame.Stop();
			}

			ImGui::Begin("Stage");

			// the game always simulates at a fixed resolution, so it's scaled
			// to fit here (letterboxed) instead of being resized to match -
			// otherwise resizing docks would distort the stage's own coordinates
			ImVec2 available = ImGui::GetContentRegionAvail();
			float scale = std::min(available.x / Editor::EmbeddedGame::kWidth, available.y / Editor::EmbeddedGame::kHeight);
			scale = std::max(scale, 0.0f);
			ImVec2 displaySize(Editor::EmbeddedGame::kWidth * scale, Editor::EmbeddedGame::kHeight * scale);

			ImVec2 cursor = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(cursor.x + (available.x - displaySize.x) * 0.5f, cursor.y + (available.y - displaySize.y) * 0.5f));

			inputAllowed = ImGui::IsWindowFocused() && !ImGui::GetIO().WantTextInput;
			// the GL texture's origin is bottom-left, but this engine's projection
			// puts world-top at the top of the image - flip V or the preview is upside down
			ImGui::Image(static_cast<ImTextureID>(embeddedGame.GetColorTextureId()), displaySize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			ImGui::End();
		}

		embeddedGame.Update(Engine::Time::GetDeltaTime(), inputAllowed);
		embeddedGame.Render();

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
