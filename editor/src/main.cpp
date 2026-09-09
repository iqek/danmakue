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
#include "SceneOverlay.h"
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
		ImVec2 panelOrigin = ImGui::GetCursorScreenPos();

		bool editing = !embeddedGame.IsPlaying();
		float margin = editing ? Editor::kSceneEditMargin : 0.0f;
		float visibleWidth = Editor::EmbeddedGame::kWidth + margin * 2.0f;
		float visibleHeight = Editor::EmbeddedGame::kHeight + margin * 2.0f;

		float scale = std::min(available.x / visibleWidth, available.y / visibleHeight);
		scale = std::max(scale, 0.0f);

		ImVec2 visibleSize(visibleWidth * scale, visibleHeight * scale);
		ImVec2 imageMin(
			panelOrigin.x + (available.x - visibleSize.x) * 0.5f + margin * scale,
			panelOrigin.y + (available.y - visibleSize.y) * 0.5f + margin * scale
		);
		ImVec2 imageMax(imageMin.x + Editor::EmbeddedGame::kWidth * scale, imageMin.y + Editor::EmbeddedGame::kHeight * scale);

		ImGui::SetCursorScreenPos(imageMin);

		bool inputAllowed = ImGui::IsWindowFocused() && !ImGui::GetIO().WantTextInput;
		// the GL texture's origin is bottom-left, but this engine's projection
		// puts world-top at the top of the image - flip V or the preview is upside down
		ImGui::Image(static_cast<ImTextureID>(embeddedGame.GetColorTextureId()), ImVec2(imageMax.x - imageMin.x, imageMax.y - imageMin.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		if(editing){
			// outline the play area, since the margin around it is only visible in the editor
			ImGui::GetWindowDrawList()->AddRect(imageMin, imageMax, IM_COL32(255, 255, 255, 60));

			Editor::SceneOverlayResult overlay = Editor::DrawSceneOverlay(stage, imageMin, scale, timelinePanel.GetSpawnVisibility(), hierarchyPanel.IsPlayerSelected(), hierarchyPanel.GetSelected(), timelinePanel.GetSelected());
			if(overlay.clickedTimelineIndex >= 0){
				hierarchyPanel.ClearSelection();
				timelinePanel.SetSelected(overlay.clickedTimelineIndex);
			}
			if(overlay.clickedPlayer){
				timelinePanel.ClearSelection();
				hierarchyPanel.SelectPlayer();
			}
		}

		ImGui::End();

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
