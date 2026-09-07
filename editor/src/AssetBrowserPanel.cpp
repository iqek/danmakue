#include "AssetBrowserPanel.h"

#include "engine/core/Utf8Path.h"

#include <algorithm>
#include <imgui.h>
#include <string>
#include <vector>

namespace Editor {

AssetBrowserPanel::AssetBrowserPanel(const std::string& utf8Root): root(Engine::PathFromUtf8(utf8Root)){}

void AssetBrowserPanel::DrawDirectory(const std::filesystem::path& directory){
	std::vector<std::filesystem::directory_entry> subdirectories;
	std::vector<std::filesystem::directory_entry> files;

	try{
		for(const auto& entry : std::filesystem::directory_iterator(directory)){
			(entry.is_directory() ? subdirectories : files).push_back(entry);
		}
	}
	catch(const std::filesystem::filesystem_error& e){
		ImGui::TextDisabled("Failed to read directory: %s", e.what());
		return;
	}

	auto byFilename = [](const auto& a, const auto& b){ return a.path().filename() < b.path().filename(); };
	std::sort(subdirectories.begin(), subdirectories.end(), byFilename);
	std::sort(files.begin(), files.end(), byFilename);

	// directories first so the listing reads like a normal file explorer
	for(const auto& entry : subdirectories){
		std::string name = entry.path().filename().string();
		if(ImGui::TreeNode(name.c_str())){
			DrawDirectory(entry.path());
			ImGui::TreePop();
		}
	}

	for(const auto& entry : files){
		std::string name = entry.path().filename().string();
		ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
	}
}

void AssetBrowserPanel::Draw(const char* title){
	ImGui::Begin(title);

	if(std::filesystem::exists(root)){
		DrawDirectory(root);
	}
	else{
		ImGui::TextDisabled("Assets folder not found: %s", root.string().c_str());
	}

	ImGui::End();
}

}
