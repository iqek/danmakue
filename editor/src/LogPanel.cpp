#include "LogPanel.h"

namespace Editor {

void LogPanel::AddLine(const std::string& line){
	int start = buffer.size();
	buffer.append(line.c_str());
	if(buffer.empty() || buffer[buffer.size() - 1] != '\n'){
		buffer.append("\n");
	}
	lineOffsets.push_back(start);
}

void LogPanel::Draw(const char* title){
	ImGui::Begin(title);

	if(ImGui::Button("Clear")){
		buffer.clear();
		lineOffsets.clear();
	}
	ImGui::SameLine();
	ImGui::Checkbox("Auto-scroll", &autoScroll);

	ImGui::Separator();
	ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	ImGuiListClipper clipper;
	clipper.Begin(lineOffsets.Size);
	while(clipper.Step()){
		for(int line = clipper.DisplayStart; line < clipper.DisplayEnd; line++){
			int lineStart = lineOffsets[line];
			int lineEnd = (line + 1 < lineOffsets.Size) ? (lineOffsets[line + 1] - 1) : buffer.size();
			ImGui::TextUnformatted(buffer.begin() + lineStart, buffer.begin() + lineEnd);
		}
	}
	clipper.End();

	if(autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()){
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::EndChild();
	ImGui::End();
}

}
