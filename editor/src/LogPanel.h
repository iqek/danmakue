#pragma once

#include <imgui.h>
#include <string>

namespace Editor {

// Ring-buffer log display, fed by LogSink attached to the engine's loggers
class LogPanel {
private:
	ImGuiTextBuffer buffer;
	ImVector<int> lineOffsets;
	bool autoScroll = true;

public:
	void AddLine(const std::string& line);
	void Draw(const char* title);
};

}
