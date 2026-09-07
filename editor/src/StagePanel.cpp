#include "StagePanel.h"

#include "engine/core/Log.h"
#include "engine/core/Utf8Path.h"

#include <algorithm>
#include <filesystem>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <vector>

namespace Editor {

namespace {

std::vector<std::string> ListStageFiles(const std::string& stagesDirectory){
	std::vector<std::string> files;

	std::filesystem::path directory = Engine::PathFromUtf8(stagesDirectory);
	if(!std::filesystem::exists(directory)){
		return files;
	}

	for(const auto& entry : std::filesystem::directory_iterator(directory)){
		if(entry.is_regular_file() && entry.path().extension() == ".json"){
			files.push_back(entry.path().filename().string());
		}
	}

	std::sort(files.begin(), files.end());
	return files;
}

}

StagePanel::StagePanel(std::string stagesDirectory, std::string initialFileName): stagesDirectory(std::move(stagesDirectory)), currentFileName(std::move(initialFileName)){}

std::string StagePanel::GetCurrentPath() const{
	return stagesDirectory + "/" + currentFileName;
}

bool StagePanel::Draw(const char* title, StageDefinition& stage){
	bool switched = false;

	ImGui::Begin(title);

	ImGui::Text("Current: %s", currentFileName.c_str());
	if(ImGui::Button("Save")){
		try{
			SaveStageDefinition(stage, GetCurrentPath());
			ENGINE_INFO("Saved stage: {}", GetCurrentPath());
		}
		catch(const std::exception& e){
			ENGINE_CORE_ERROR("Failed to save stage {}: {}", GetCurrentPath(), e.what());
		}
	}

	ImGui::Separator();

	std::vector<std::string> stageFiles = ListStageFiles(stagesDirectory);
	if(ImGui::BeginCombo("Stage", currentFileName.c_str())){
		for(const auto& fileName : stageFiles){
			if(ImGui::Selectable(fileName.c_str(), fileName == currentFileName)){
				currentFileName = fileName;
				switched = true;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	ImGui::InputText("New Stage Name", &newStageName);
	ImGui::SameLine();
	if(ImGui::Button("Create")){
		if(newStageName.empty()){
			ENGINE_CORE_ERROR("New stage name can't be empty");
		}
		else{
			std::string fileName = newStageName + ".json";
			std::string path = stagesDirectory + "/" + fileName;

			if(std::filesystem::exists(Engine::PathFromUtf8(path))){
				ENGINE_CORE_ERROR("Stage already exists: {}", fileName);
			}
			else{
				try{
					SaveStageDefinition(StageDefinition{}, path);
					ENGINE_INFO("Created stage: {}", path);
					currentFileName = fileName;
					newStageName.clear();
					switched = true;
				}
				catch(const std::exception& e){
					ENGINE_CORE_ERROR("Failed to create stage {}: {}", path, e.what());
				}
			}
		}
	}

	ImGui::End();

	return switched;
}

}
