#pragma once

#include "StageDefinition.h"

#include <string>

namespace Editor {

// Picks which stage file is open, creates new ones, and saves the current one
class StagePanel {
private:
	std::string stagesDirectory;
	std::string currentFileName;
	std::string newStageName;

public:
	StagePanel(std::string stagesDirectory, std::string initialFileName);

	const std::string& GetCurrentFileName() const { return currentFileName; }
	std::string GetCurrentPath() const;

	// returns true if a different stage was selected or created this frame -
	// the caller is responsible for reloading StageDefinition in that case
	bool Draw(const char* title, StageDefinition& stage);
};

}
