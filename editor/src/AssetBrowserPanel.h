#pragma once

#include <filesystem>
#include <string>

namespace Editor {

// Read-only directory tree over a project folder, e.g. the game's assets
class AssetBrowserPanel {
private:
	std::filesystem::path root;

	void DrawDirectory(const std::filesystem::path& directory);

public:
	explicit AssetBrowserPanel(const std::string& utf8Root);

	void Draw(const char* title);
};

}
