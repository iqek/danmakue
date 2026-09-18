#include "TimelinePanel.h"
#include "JsonFieldHelpers.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <imgui.h>
#include <string>
#include <utility>

namespace Editor {

namespace {

using Json = nlohmann::json;

constexpr float kLaneHeight = 22.0f;
constexpr float kLaneSpacing = 4.0f;
constexpr float kMinBlockWidth = 18.0f;
constexpr float kMaxBlockWidth = 130.0f;
constexpr float kBlockGap = 4.0f;
// clear space around a gate, so the break in the clock reads as a break
constexpr float kSegmentGap = 46.0f;
constexpr float kRulerHeight = 18.0f;
constexpr float kPhaseBandHeight = 16.0f;
constexpr int kMaxVisibleLanes = 4;
// how close a dragged block has to get to another instant before it snaps onto it
constexpr float kSnapPixels = 7.0f;

const ImVec4 kWarningColor(1.0f, 0.75f, 0.25f, 1.0f);
const ImU32 kGateColor = IM_COL32(120, 200, 255, 230);

// every structural edit is deferred, so it can't resize the vector being walked
struct PendingActions {
	int duplicate = -1;
	int remove = -1;
	// a drag: the entry takes the instant of the row it landed on
	int moveFrom = -1;
	int moveTo = -1;
	// the menu's earlier/later: trade places with a neighbour, leaving the schedule alone
	int swapFrom = -1;
	int swapTo = -1;
	int retime = -1;
	float retimeTime = 0.0f;
	// a dragged gate has no time to land on, so it only changes place in the order
	int reorderFrom = -1;
	int reorderTo = -1;
	bool insert = false;
	int insertIndex = 0;
	float insertDelay = 0.0f;
	std::string insertSpawnId;

	bool Any() const {
		return duplicate >= 0 || remove >= 0 || moveFrom >= 0 || swapFrom >= 0 || retime >= 0 || reorderFrom >= 0 || insert;
	}
};

struct TrackGeometry {
	std::vector<float> x;
	std::vector<float> width;
	std::vector<float> segmentStart;
	std::vector<float> segmentSpan;
	float contentWidth = 0.0f;
	int maxLane = 0;
};

bool IsGate(const Json& trigger){
	return trigger.value("type", std::string("delay")) == "afterCleared";
}

float GetDelay(const Json& trigger){
	return IsGate(trigger) ? 0.0f : std::max(0.0f, trigger.value("seconds", 0.0f));
}

void SetDelay(Json& trigger, float seconds){
	SetFloat(trigger, "seconds", std::max(0.0f, seconds));
}

Json MakeDelayTrigger(float seconds){
	Json trigger;
	trigger["type"] = "delay";
	SetDelay(trigger, seconds);
	return trigger;
}

// trims the zeros a fixed format leaves behind, so 1.00 reads as 1s
std::string FormatSeconds(float seconds){
	char buffer[32];
	std::snprintf(buffer, sizeof(buffer), "%.2f", seconds);
	std::string text = buffer;
	while(text.find('.') != std::string::npos && (text.back() == '0' || text.back() == '.')){
		bool wasDot = text.back() == '.';
		text.pop_back();
		if(wasDot){
			break;
		}
	}
	return text + "s";
}

std::string WaitLabel(const Json& trigger){
	if(IsGate(trigger)){
		return "until cleared";
	}
	float delay = GetDelay(trigger);
	return delay <= 0.0f ? "same instant" : "wait " + FormatSeconds(delay);
}

// segment zero runs on the stage clock, later ones count from their gate
std::string TimeLabel(const TimelineSlot& slot){
	return (slot.segment == 0 ? "" : "+") + FormatSeconds(slot.time);
}

ImU32 PhaseColorU32(const StageDefinition& stage, const std::string& phase, float alpha){
	glm::vec4 color = PhaseColorOf(stage, phase);
	return ImGui::GetColorU32(ImVec4(color.r, color.g, color.b, color.a * alpha));
}

// dark text on a light phase and light text on a dark one, since the colour is the user's to pick
ImU32 PhaseTextColor(const StageDefinition& stage, const std::string& phase){
	glm::vec4 color = PhaseColorOf(stage, phase);
	float luminance = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
	return luminance > 0.55f ? IM_COL32(20, 20, 24, 255) : IM_COL32(244, 244, 248, 255);
}

// the coarsest step that still leaves about a label's width between ticks
float TickStep(float pixelsPerSecond){
	static const float steps[] = { 0.25f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 15.0f, 30.0f, 60.0f, 300.0f };
	for(float step : steps){
		if(step * pixelsPerSecond >= 56.0f){
			return step;
		}
	}
	return steps[IM_ARRAYSIZE(steps) - 1];
}

TrackGeometry BuildGeometry(const StageDefinition& stage, const std::vector<TimelineSlot>& slots, float pixelsPerSecond){
	TrackGeometry geometry;
	int count = static_cast<int>(slots.size());
	int segmentCount = count > 0 ? slots[count - 1].segment + 1 : 1;

	geometry.x.assign(count, 0.0f);
	geometry.width.assign(count, kMinBlockWidth);
	geometry.segmentStart.assign(segmentCount, 0.0f);
	geometry.segmentSpan.assign(segmentCount, 0.0f);

	// a block would rather be as wide as its label, and is trimmed back later
	std::vector<float> preferred(count, kMinBlockWidth);
	for(int i = 0; i < count; i++){
		float textWidth = ImGui::CalcTextSize(stage.timeline[i].spawnId.c_str()).x;
		preferred[i] = std::clamp(textWidth + 14.0f, kMinBlockWidth, kMaxBlockWidth);
		geometry.segmentSpan[slots[i].segment] = std::max(geometry.segmentSpan[slots[i].segment], slots[i].time * pixelsPerSecond + preferred[i]);
		geometry.maxLane = std::max(geometry.maxLane, slots[i].lane);
	}

	float cursor = 0.0f;
	for(int s = 0; s < segmentCount; s++){
		if(s > 0){
			cursor += kSegmentGap;
		}
		geometry.segmentStart[s] = cursor;
		cursor += std::max(geometry.segmentSpan[s], kSegmentGap);
	}
	geometry.contentWidth = cursor;

	for(int i = 0; i < count; i++){
		geometry.x[i] = geometry.segmentStart[slots[i].segment] + slots[i].time * pixelsPerSecond;
		geometry.width[i] = preferred[i];
	}

	// lanes never overlap vertically, so a block only gives way to the next one in its own
	for(int i = 0; i < count; i++){
		for(int j = i + 1; j < count; j++){
			if(slots[j].lane != slots[i].lane){
				continue;
			}
			geometry.width[i] = std::clamp(geometry.x[j] - geometry.x[i] - kBlockGap, kMinBlockWidth, geometry.width[i]);
			break;
		}
	}

	return geometry;
}

// where a drop at this point on the track belongs in the firing order
int FindInsertIndex(const std::vector<TimelineSlot>& slots, int segment, float time){
	int index = 0;
	for(int i = 0; i < static_cast<int>(slots.size()); i++){
		if(slots[i].segment < segment){
			index = i + 1;
		}
		else if(slots[i].segment > segment){
			break;
		}
		else if(slots[i].time <= time){
			index = i + 1;
		}
	}
	return index;
}

int SegmentAt(const TrackGeometry& geometry, float localX){
	int segment = 0;
	for(int s = 0; s < static_cast<int>(geometry.segmentStart.size()); s++){
		if(localX >= geometry.segmentStart[s]){
			segment = s;
		}
	}
	return segment;
}

// landing exactly on another entry is what stacks them, so pull the last few pixels shut
float SnapTime(const std::vector<TimelineSlot>& slots, int index, float time, float pixelsPerSecond){
	float best = std::max(0.0f, time);
	float closest = kSnapPixels / pixelsPerSecond;

	if(best < closest){
		return 0.0f;
	}

	for(int i = 0; i < static_cast<int>(slots.size()); i++){
		if(i == index || slots[i].segment != slots[index].segment){
			continue;
		}
		float distance = std::abs(slots[i].time - time);
		if(distance < closest){
			closest = distance;
			best = slots[i].time;
		}
	}

	return best;
}

// how far along the order a dragged gate has been pulled, ignoring its own block
int GateDropIndex(const std::vector<TimelineSlot>& slots, const TrackGeometry& geometry, float localX, int ignore){
	int index = 0;
	for(int i = 0; i < static_cast<int>(slots.size()); i++){
		if(i != ignore && geometry.x[i] + geometry.width[i] * 0.5f < localX){
			index++;
		}
	}
	return index;
}

// a fresh entry lands where the one before it does, or just above the player
glm::vec2 DefaultSpawnPosition(const StageDefinition& stage, int insertIndex){
	if(insertIndex > 0 && insertIndex <= static_cast<int>(stage.timeline.size())){
		return stage.timeline[insertIndex - 1].position;
	}
	return glm::vec2(stage.player.position.x, -40.0f);
}

bool SpawnIsMissing(const StageDefinition& stage, const TimelineEntry& entry){
	return FindEnemy(stage, entry.spawnId) == nullptr;
}

// joins the instant of the entry already at insertIndex - 1
void QueueDrop(const StageDefinition& stage, PendingActions& actions, int enemyIndex, int insertIndex, float delay){
	if(enemyIndex < 0 || enemyIndex >= static_cast<int>(stage.enemies.size())){
		return;
	}
	actions.insert = true;
	actions.insertIndex = insertIndex;
	actions.insertDelay = delay;
	actions.insertSpawnId = stage.enemies[enemyIndex].id;
}

// turns a point along the track into an insert request at that moment
void QueueInsertAt(const StageDefinition& stage, const std::vector<TimelineSlot>& slots, const TrackGeometry& geometry, PendingActions& actions, float localX, float pixelsPerSecond, const std::string& spawnId){
	if(spawnId.empty()){
		return;
	}

	int segment = SegmentAt(geometry, localX);
	float time = std::max(0.0f, (localX - geometry.segmentStart[segment]) / pixelsPerSecond);
	int insertIndex = FindInsertIndex(slots, segment, time);
	float previousTime = (insertIndex > 0 && slots[insertIndex - 1].segment == segment) ? slots[insertIndex - 1].time : 0.0f;

	actions.insert = true;
	actions.insertIndex = insertIndex;
	actions.insertDelay = time - previousTime;
	actions.insertSpawnId = spawnId;
}

void DrawEntryContextMenu(StageDefinition& stage, int index, PendingActions& actions){
	if(!ImGui::BeginPopupContextItem()){
		return;
	}
	if(ImGui::MenuItem("Duplicate Into This Instant")){
		actions.duplicate = index;
	}
	if(ImGui::MenuItem("Delete")){
		actions.remove = index;
	}
	ImGui::Separator();
	if(ImGui::MenuItem("Swap With Earlier", nullptr, false, index > 0)){
		actions.swapFrom = index;
		actions.swapTo = index - 1;
	}
	if(ImGui::MenuItem("Swap With Later", nullptr, false, index + 1 < static_cast<int>(stage.timeline.size()))){
		actions.swapFrom = index;
		actions.swapTo = index + 1;
	}
	ImGui::Separator();
	if(ImGui::MenuItem("Wait For Screen Clear", nullptr, IsGate(stage.timeline[index].trigger))){
		if(IsGate(stage.timeline[index].trigger)){
			stage.timeline[index].trigger = MakeDelayTrigger(1.0f);
		}
		else{
			stage.timeline[index].trigger = Json{ { "type", "afterCleared" } };
		}
	}
	ImGui::EndPopup();
}

void DrawEntryTooltip(const StageDefinition& stage, const TimelineSlot& slot, int index){
	ImGui::BeginTooltip();
	ImGui::Text("%d: %s", index, stage.timeline[index].spawnId.c_str());
	ImGui::TextDisabled("%s", WaitLabel(stage.timeline[index].trigger).c_str());
	if(slot.segment == 0){
		ImGui::TextDisabled("spawns %s into the stage", TimeLabel(slot).c_str());
	}
	else{
		ImGui::TextDisabled("spawns %s after the screen clears", FormatSeconds(slot.time).c_str());
	}
	if(slot.emptyGate){
		ImGui::TextColored(kWarningColor, "nothing has spawned yet, so this clears at once");
	}
	if(SpawnIsMissing(stage, stage.timeline[index])){
		ImGui::TextColored(kWarningColor, "no enemy has this id any more, so nothing will spawn");
	}
	const std::string& phase = EntryPhase(stage, stage.timeline[index]);
	if(!phase.empty()){
		ImGui::TextDisabled("phase: %s", phase.c_str());
	}
	ImGui::EndTooltip();
}

void DrawRulers(ImDrawList* drawList, const TrackGeometry& geometry, const ImVec2& origin, float pixelsPerSecond, float lanesBottom){
	float step = TickStep(pixelsPerSecond);

	for(int s = 0; s < static_cast<int>(geometry.segmentStart.size()); s++){
		float span = geometry.segmentSpan[s];
		for(float t = 0.0f; t * pixelsPerSecond <= span; t += step){
			float x = origin.x + geometry.segmentStart[s] + t * pixelsPerSecond;
			drawList->AddLine(ImVec2(x, origin.y + kRulerHeight - 4.0f), ImVec2(x, lanesBottom), ImGui::GetColorU32(ImGuiCol_Border, 0.55f));
			std::string label = (s == 0 ? "" : "+") + FormatSeconds(t);
			drawList->AddText(ImVec2(x + 3.0f, origin.y), ImGui::GetColorU32(ImGuiCol_TextDisabled), label.c_str());
		}
	}
}

void DrawPhaseBands(ImDrawList* drawList, const StageDefinition& stage, const std::vector<TimelineSlot>& slots, const TrackGeometry& geometry, const ImVec2& origin, float lanesBottom){
	int count = static_cast<int>(slots.size());

	// a divider only where the phase really changes, skipping stacks that share one instant
	for(int i = 1; i < count; i++){
		if(slots[i].lane != 0 || EntryPhase(stage, stage.timeline[i]) == EntryPhase(stage, stage.timeline[i - 1])){
			continue;
		}
		float x = origin.x + geometry.x[i] - kBlockGap * 0.5f;
		drawList->AddLine(ImVec2(x, origin.y + kRulerHeight), ImVec2(x, lanesBottom), ImGui::GetColorU32(ImGuiCol_Border, 0.9f), 2.0f);
	}

	for(int start = 0; start < count; ){
		const std::string& phase = EntryPhase(stage, stage.timeline[start]);
		int end = start;
		while(end + 1 < count && EntryPhase(stage, stage.timeline[end + 1]) == phase && slots[end + 1].segment == slots[start].segment){
			end++;
		}

		if(!phase.empty()){
			ImVec2 bandMin(origin.x + geometry.x[start], origin.y + kRulerHeight);
			ImVec2 bandMax(origin.x + geometry.x[end] + geometry.width[end], bandMin.y + kPhaseBandHeight - 3.0f);
			drawList->AddRectFilled(bandMin, bandMax, PhaseColorU32(stage, phase, 0.5f), 3.0f);
			drawList->PushClipRect(bandMin, bandMax, true);
			drawList->AddText(ImVec2(bandMin.x + 5.0f, bandMin.y), PhaseTextColor(stage, phase), phase.c_str());
			drawList->PopClipRect();
		}

		start = end + 1;
	}
}

void DrawTrack(StageDefinition& stage, const std::vector<TimelineSlot>& slots, const TrackGeometry& geometry, int& selectedIndex, float& pixelsPerSecond, int& draggingIndex, float& dragTime, PendingActions& actions, bool& changed){
	float laneStride = kLaneHeight + kLaneSpacing;
	float lanesHeight = (geometry.maxLane + 1) * laneStride;
	float visibleLanes = std::min(static_cast<float>(kMaxVisibleLanes), static_cast<float>(geometry.maxLane + 1));
	float childHeight = kRulerHeight + kPhaseBandHeight + visibleLanes * laneStride + 16.0f;

	ImGui::BeginChild("track", ImVec2(0.0f, childHeight), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);

	ImVec2 origin = ImGui::GetCursorScreenPos();
	float contentWidth = std::max(geometry.contentWidth, ImGui::GetContentRegionAvail().x);
	float lanesTop = origin.y + kRulerHeight + kPhaseBandHeight;
	float lanesBottom = lanesTop + lanesHeight;

	// a background item both sizes the scroll region and catches enemies dragged from the Hierarchy.
	// without AllowOverlap it would swallow every click, leaving the blocks on top of it dead
	ImGui::SetNextItemAllowOverlap();
	ImGui::InvisibleButton("##background", ImVec2(contentWidth, kRulerHeight + kPhaseBandHeight + lanesHeight));
	if(ImGui::BeginDragDropTarget()){
		if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENEMY_ROW")){
			int dragged = *static_cast<const int*>(payload->Data);
			if(dragged >= 0 && dragged < static_cast<int>(stage.enemies.size())){
				QueueInsertAt(stage, slots, geometry, actions, ImGui::GetIO().MousePos.x - origin.x, pixelsPerSecond, stage.enemies[dragged].id);
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	DrawRulers(drawList, geometry, origin, pixelsPerSecond, lanesBottom);
	DrawPhaseBands(drawList, stage, slots, geometry, origin, lanesBottom);

	int count = static_cast<int>(slots.size());
	for(int i = 0; i < count; i++){
		ImGui::PushID(i);

		bool dragging = draggingIndex == i;
		bool gate = slots[i].isGate;
		float blockWidth = dragging ? std::max(geometry.width[i], kMinBlockWidth) : geometry.width[i];

		float shownTime = dragging && !gate ? dragTime : slots[i].time;
		float blockX = geometry.segmentStart[slots[i].segment] + shownTime * pixelsPerSecond;
		// a gate has no time to slide along, so a dragged one just follows the cursor
		if(dragging && gate){
			blockX = ImGui::GetIO().MousePos.x - origin.x - blockWidth * 0.5f;
		}

		ImVec2 blockMin(origin.x + blockX, lanesTop + slots[i].lane * laneStride);
		ImVec2 blockMax(blockMin.x + blockWidth, blockMin.y + kLaneHeight);

		// the tie line makes a stack read as one instant rather than separate rows
		if(slots[i].sharesInstant && !dragging){
			drawList->AddLine(ImVec2(blockMin.x + 1.0f, blockMin.y - kLaneSpacing), ImVec2(blockMin.x + 1.0f, blockMin.y), ImGui::GetColorU32(ImGuiCol_Text, 0.5f), 2.0f);
		}

		ImGui::SetCursorScreenPos(blockMin);
		ImGui::InvisibleButton("##block", ImVec2(blockWidth, kLaneHeight));

		if(ImGui::IsItemActivated()){
			selectedIndex = i;
			changed = true;
			draggingIndex = i;
			dragTime = slots[i].time;
			dragging = true;
		}

		if(dragging && ImGui::IsItemActive()){
			dragTime = std::max(0.0f, dragTime + ImGui::GetIO().MouseDelta.x / pixelsPerSecond);
		}

		// committed only on release, so the order never changes under an active drag
		if(dragging && ImGui::IsItemDeactivated()){
			if(gate){
				int target = GateDropIndex(slots, geometry, ImGui::GetIO().MousePos.x - origin.x, i);
				if(target != i){
					actions.reorderFrom = i;
					actions.reorderTo = target;
				}
			}
			else{
				float snapped = SnapTime(slots, i, dragTime, pixelsPerSecond);
				if(std::abs(snapped - slots[i].time) > 0.0005f){
					actions.retime = i;
					actions.retimeTime = snapped;
				}
			}
			draggingIndex = -1;
		}

		// an enemy dropped straight onto a block joins that exact instant
		if(ImGui::BeginDragDropTarget()){
			if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENEMY_ROW")){
				QueueDrop(stage, actions, *static_cast<const int*>(payload->Data), i + 1, 0.0f);
			}
			ImGui::EndDragDropTarget();
		}

		const std::string& phase = EntryPhase(stage, stage.timeline[i]);
		bool phased = !phase.empty();
		bool missing = SpawnIsMissing(stage, stage.timeline[i]);
		ImU32 fill = ImGui::GetColorU32(ImGuiCol_Button);
		ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
		if(missing){
			fill = ImGui::GetColorU32(ImVec4(0.55f, 0.20f, 0.22f, 0.95f));
			textColor = IM_COL32(255, 215, 215, 255);
		}
		else if(phased){
			fill = PhaseColorU32(stage, phase, 0.9f);
			textColor = PhaseTextColor(stage, phase);
		}
		drawList->AddRectFilled(blockMin, blockMax, fill, 3.0f);

		if(slots[i].isGate){
			drawList->AddRectFilled(blockMin, ImVec2(blockMin.x + 3.0f, blockMax.y), kGateColor);
		}
		if(slots[i].emptyGate){
			drawList->AddRectFilled(blockMin, ImVec2(blockMin.x + 3.0f, blockMax.y), ImGui::GetColorU32(kWarningColor));
		}
		if(dragging || selectedIndex == i){
			drawList->AddRect(blockMin, blockMax, ImGui::GetColorU32(ImGuiCol_Text), 3.0f, 0, 2.0f);
		}
		else if(ImGui::IsItemHovered()){
			drawList->AddRect(blockMin, blockMax, ImGui::GetColorU32(ImGuiCol_Border), 3.0f);
		}

		drawList->PushClipRect(blockMin, blockMax, true);
		drawList->AddText(ImVec2(blockMin.x + 6.0f, blockMin.y + 3.0f), textColor, stage.timeline[i].spawnId.c_str());
		drawList->PopClipRect();

		// a guide line reads the new instant off the ruler while the block is in flight
		if(dragging){
			drawList->AddLine(ImVec2(blockMin.x, origin.y + kRulerHeight), ImVec2(blockMin.x, lanesBottom), ImGui::GetColorU32(kWarningColor), 1.5f);
			std::string label = (slots[i].segment == 0 ? "" : "+") + FormatSeconds(dragTime);
			drawList->AddText(ImVec2(blockMin.x + 3.0f, origin.y), ImGui::GetColorU32(kWarningColor), label.c_str());
		}
		else if(ImGui::IsItemHovered()){
			DrawEntryTooltip(stage, slots[i], i);
		}

		DrawEntryContextMenu(stage, i, actions);

		ImGui::PopID();
	}

	if(count == 0){
		drawList->AddText(ImVec2(origin.x + 8.0f, lanesTop + 4.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), "drop an enemy here, or right click to add one");
	}

	// only fires over empty track, so it doesn't fight the per-block menus
	if(ImGui::BeginPopupContextWindow("trackContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)){
		if(ImGui::MenuItem("New Entry Here", nullptr, false, !stage.enemies.empty())){
			// the spot that was right clicked, not wherever the mouse drifted in the menu
			QueueInsertAt(stage, slots, geometry, actions, ImGui::GetMousePosOnOpeningCurrentPopup().x - origin.x, pixelsPerSecond, stage.enemies[0].id);
		}
		ImGui::EndPopup();
	}

	if(ImGui::IsWindowHovered() && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0.0f){
		pixelsPerSecond = std::clamp(pixelsPerSecond * (1.0f + ImGui::GetIO().MouseWheel * 0.12f), 8.0f, 400.0f);
	}

	ImGui::EndChild();
}

void DrawEntryTable(StageDefinition& stage, const std::vector<TimelineSlot>& slots, int& selectedIndex, PendingActions& actions, bool& changed){
	ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingStretchProp;
	if(!ImGui::BeginTable("entries", 5, flags)){
		return;
	}

	ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 22.0f);
	ImGui::TableSetupColumn("Wait", ImGuiTableColumnFlags_WidthFixed, 90.0f);
	ImGui::TableSetupColumn("At", ImGuiTableColumnFlags_WidthFixed, 52.0f);
	ImGui::TableSetupColumn("Spawn");
	ImGui::TableSetupColumn("Phase");
	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableHeadersRow();

	for(int i = 0; i < static_cast<int>(stage.timeline.size()); i++){
		ImGui::PushID(i);
		ImGui::TableNextRow();

		const std::string& phase = EntryPhase(stage, stage.timeline[i]);
		if(!phase.empty()){
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, PhaseColorU32(stage, phase, 0.16f));
		}

		ImGui::TableSetColumnIndex(0);
		char index[8];
		std::snprintf(index, sizeof(index), "%d", i);
		if(ImGui::Selectable(index, selectedIndex == i, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)){
			selectedIndex = i;
			changed = true;
		}

		// dropping a row on another gives it that row's moment, same as dragging on the track
		if(ImGui::BeginDragDropSource()){
			ImGui::SetDragDropPayload("TIMELINE_ROW", &i, sizeof(int));
			ImGui::TextUnformatted(stage.timeline[i].spawnId.c_str());
			ImGui::EndDragDropSource();
		}
		if(ImGui::BeginDragDropTarget()){
			if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TIMELINE_ROW")){
				actions.moveFrom = *static_cast<const int*>(payload->Data);
				actions.moveTo = i;
			}
			// an enemy dropped on a row spawns alongside it, same as on the track
			if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENEMY_ROW")){
				QueueDrop(stage, actions, *static_cast<const int*>(payload->Data), i + 1, 0.0f);
			}
			ImGui::EndDragDropTarget();
		}

		DrawEntryContextMenu(stage, i, actions);

		ImGui::TableSetColumnIndex(1);
		if(slots[i].isGate){
			ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(kGateColor), "until cleared");
		}
		else if(slots[i].sharesInstant){
			ImGui::TextDisabled("same instant");
		}
		else{
			ImGui::TextUnformatted(FormatSeconds(GetDelay(stage.timeline[i].trigger)).c_str());
		}

		ImGui::TableSetColumnIndex(2);
		ImGui::TextUnformatted(TimeLabel(slots[i]).c_str());

		ImGui::TableSetColumnIndex(3);
		if(SpawnIsMissing(stage, stage.timeline[i])){
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "%s (missing)", stage.timeline[i].spawnId.c_str());
		}
		else{
			ImGui::TextUnformatted(stage.timeline[i].spawnId.c_str());
		}

		ImGui::TableSetColumnIndex(4);
		if(phase.empty()){
			ImGui::TextDisabled("-");
		}
		else{
			// a solid cell turns a run of one phase into an unbroken bar down the column
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, PhaseColorU32(stage, phase, 0.85f));
			ImGui::PushStyleColor(ImGuiCol_Text, PhaseTextColor(stage, phase));
			ImGui::TextUnformatted(phase.c_str());
			ImGui::PopStyleColor();
		}

		ImGui::PopID();
	}

	ImGui::EndTable();
}

void DrawHelpMarker(){
	ImGui::TextDisabled("(?)");
	if(!ImGui::IsItemHovered()){
		return;
	}
	ImGui::BeginTooltip();
	ImGui::TextUnformatted("A wait counts from the previous spawn, not from the start.");
	ImGui::TextUnformatted("A wait of zero means at the same time, stacked in the lane above.");
	ImGui::TextUnformatted("A clear gate stops the clock, so the ruler restarts after it.");
	ImGui::Separator();
	ImGui::TextUnformatted("Dragging always means: put this at that moment.");
	ImGui::TextUnformatted("Drag a block sideways to a new time, or a row onto another row.");
	ImGui::TextUnformatted("A gate has no time, so dragging one moves it along the order instead.");
	ImGui::TextUnformatted("Drag an enemy in from the Hierarchy to spawn it where you drop it.");
	ImGui::TextUnformatted("Right click a row to swap it with a neighbour instead.");
	ImGui::TextUnformatted("Phases come from the enemy, so tag it in the Hierarchy.");
	ImGui::TextUnformatted("Ctrl+wheel zooms.");
	ImGui::Separator();
	ImGui::TextUnformatted("The scene shows the paths of whatever is selected.");
	ImGui::TextUnformatted("Pick a phase to lay all of its paths out together.");
	ImGui::EndTooltip();
}

// a dragged entry takes the instant of the one it landed on, which is what the track drag does
void MoveToInstant(StageDefinition& stage, int from, int to, int& selectedIndex){
	TimelineEntry moved = stage.timeline[from];

	// hand its wait to whatever followed it, so the rest of that stretch keeps its timing
	if(from + 1 < static_cast<int>(stage.timeline.size()) && !IsGate(stage.timeline[from].trigger) && !IsGate(stage.timeline[from + 1].trigger)){
		SetDelay(stage.timeline[from + 1].trigger, GetDelay(stage.timeline[from + 1].trigger) + GetDelay(stage.timeline[from].trigger));
	}

	stage.timeline.erase(stage.timeline.begin() + from);

	// a zero wait right behind the target is what puts the two in one instant
	if(!IsGate(moved.trigger)){
		moved.trigger = MakeDelayTrigger(0.0f);
	}

	int insertIndex = to > from ? to : to + 1;
	stage.timeline.insert(stage.timeline.begin() + insertIndex, moved);
	selectedIndex = insertIndex;
}

// a gate carries its own trigger, so moving one is a plain change of place
void ReorderEntry(StageDefinition& stage, int from, int to){
	TimelineEntry moved = stage.timeline[from];
	stage.timeline.erase(stage.timeline.begin() + from);
	stage.timeline.insert(stage.timeline.begin() + to, moved);
}

// swapping leaves every wait where it was, so only the order of spawns changes
void MoveEntry(StageDefinition& stage, int from, int to){
	std::vector<Json> triggers;
	triggers.reserve(stage.timeline.size());
	for(const auto& entry : stage.timeline){
		triggers.push_back(entry.trigger);
	}

	TimelineEntry moved = stage.timeline[from];
	stage.timeline.erase(stage.timeline.begin() + from);
	stage.timeline.insert(stage.timeline.begin() + to, moved);

	for(std::size_t i = 0; i < stage.timeline.size(); i++){
		stage.timeline[i].trigger = triggers[i];
	}
}

void ApplyPendingActions(StageDefinition& stage, const std::vector<TimelineSlot>& slots, const PendingActions& actions, int& selectedIndex, bool& changed){
	int count = static_cast<int>(stage.timeline.size());

	if(actions.insert && actions.insertIndex >= 0 && actions.insertIndex <= count){
		TimelineEntry entry;
		entry.spawnId = actions.insertSpawnId;
		entry.position = DefaultSpawnPosition(stage, actions.insertIndex);

		entry.trigger = MakeDelayTrigger(actions.insertDelay);

		// take the new wait out of the next one, so nothing later moves
		if(actions.insertIndex < count && !IsGate(stage.timeline[actions.insertIndex].trigger)){
			SetDelay(stage.timeline[actions.insertIndex].trigger, GetDelay(stage.timeline[actions.insertIndex].trigger) - actions.insertDelay);
		}

		stage.timeline.insert(stage.timeline.begin() + actions.insertIndex, entry);
		selectedIndex = actions.insertIndex;
		changed = true;
		return;
	}

	if(actions.duplicate >= 0 && actions.duplicate < count){
		TimelineEntry copy = stage.timeline[actions.duplicate];
		// a zero wait is what puts the copy in the same instant as its source
		copy.trigger = MakeDelayTrigger(0.0f);
		// nudge it, or the copy's scene marker sits exactly under the original's
		copy.position.x += 30.0f;
		copy.position.y += 30.0f;
		stage.timeline.insert(stage.timeline.begin() + actions.duplicate + 1, copy);
		selectedIndex = actions.duplicate + 1;
		changed = true;
		return;
	}

	if(actions.retime >= 0 && actions.retime < count && actions.retime < static_cast<int>(slots.size())){
		RetimeEntry(stage, slots, actions.retime, actions.retimeTime, selectedIndex);
		changed = true;
		return;
	}

	if(actions.moveFrom >= 0 && actions.moveFrom < count && actions.moveTo >= 0 && actions.moveTo < count && actions.moveFrom != actions.moveTo){
		MoveToInstant(stage, actions.moveFrom, actions.moveTo, selectedIndex);
		changed = true;
		return;
	}

	if(actions.reorderFrom >= 0 && actions.reorderFrom < count && actions.reorderTo >= 0 && actions.reorderTo < count && actions.reorderFrom != actions.reorderTo){
		ReorderEntry(stage, actions.reorderFrom, actions.reorderTo);
		selectedIndex = actions.reorderTo;
		changed = true;
		return;
	}

	if(actions.swapFrom >= 0 && actions.swapFrom < count && actions.swapTo >= 0 && actions.swapTo < count && actions.swapFrom != actions.swapTo){
		MoveEntry(stage, actions.swapFrom, actions.swapTo);
		selectedIndex = actions.swapTo;
		changed = true;
		return;
	}

	if(actions.remove >= 0 && actions.remove < count){
		// hand the removed wait to the next entry, so the rest of the stage keeps its timing
		if(actions.remove + 1 < count && !IsGate(stage.timeline[actions.remove].trigger) && !IsGate(stage.timeline[actions.remove + 1].trigger)){
			SetDelay(stage.timeline[actions.remove + 1].trigger, GetDelay(stage.timeline[actions.remove + 1].trigger) + GetDelay(stage.timeline[actions.remove].trigger));
		}

		stage.timeline.erase(stage.timeline.begin() + actions.remove);
		if(selectedIndex == actions.remove){
			selectedIndex = -1;
		}
		else if(selectedIndex > actions.remove){
			selectedIndex--;
		}
	}
}

}

// Rewrites one segment from absolute times, which is what lets a drag reorder it.
// Entries keep their own instants, so nothing outside the dragged one moves.
void RetimeEntry(StageDefinition& stage, const std::vector<TimelineSlot>& slots, int index, float time, int& selectedIndex){
	int segment = slots[index].segment;

	std::vector<std::pair<float, int>> ordered;
	int first = -1;
	for(int i = 0; i < static_cast<int>(slots.size()); i++){
		if(slots[i].segment != segment || slots[i].isGate){
			continue;
		}
		if(first < 0){
			first = i;
		}
		ordered.push_back({ i == index ? time : slots[i].time, i });
	}

	if(first < 0){
		return;
	}

	std::stable_sort(ordered.begin(), ordered.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b){
		return a.first < b.first;
	});

	std::vector<TimelineEntry> rebuilt;
	rebuilt.reserve(ordered.size());
	float previous = 0.0f;
	for(std::size_t slot = 0; slot < ordered.size(); slot++){
		TimelineEntry entry = stage.timeline[ordered[slot].second];
		SetDelay(entry.trigger, ordered[slot].first - previous);
		previous = ordered[slot].first;
		if(ordered[slot].second == index){
			selectedIndex = first + static_cast<int>(slot);
		}
		rebuilt.push_back(std::move(entry));
	}

	for(std::size_t slot = 0; slot < rebuilt.size(); slot++){
		stage.timeline[first + slot] = rebuilt[slot];
	}
}

std::vector<TimelineSlot> ComputeTimelineLayout(const StageDefinition& stage){
	std::vector<TimelineSlot> slots;
	slots.reserve(stage.timeline.size());

	float cursor = 0.0f;
	int segment = 0;
	int lane = 0;

	for(const auto& entry : stage.timeline){
		TimelineSlot slot;

		if(IsGate(entry.trigger)){
			// a gate ends the segment before it, since how long it takes is up to the player
			segment++;
			cursor = 0.0f;
			lane = 0;
			slot.isGate = true;
			slot.emptyGate = slots.empty();
		}
		else{
			float seconds = GetDelay(entry.trigger);
			slot.sharesInstant = !slots.empty() && seconds <= 0.0f;
			cursor += seconds;
			lane = slot.sharesInstant ? lane + 1 : 0;
		}

		slot.segment = segment;
		slot.time = cursor;
		slot.lane = lane;
		slots.push_back(slot);
	}

	return slots;
}

bool TimelinePanel::Draw(const char* title, StageDefinition& stage){
	bool changed = false;
	PendingActions actions;

	ImGui::Begin(title);

	ImGui::TextDisabled("Show all");
	ImGui::SameLine();
	ImGui::Checkbox("spawn points", &viewOptions.showAllSpawns);
	if(ImGui::IsItemHovered()){
		ImGui::SetTooltip("Off leaves only the markers the selection covers");
	}
	ImGui::SameLine();
	ImGui::Checkbox("paths", &viewOptions.showAllPaths);
	if(ImGui::IsItemHovered()){
		ImGui::SetTooltip("Every path in the stage at once, however crowded it gets");
	}
	ImGui::SameLine();
	ImGui::TextDisabled("Zoom");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(110.0f);
	ImGui::SliderFloat("##zoom", &pixelsPerSecond, 8.0f, 400.0f, "%.0f px/s", ImGuiSliderFlags_Logarithmic);
	ImGui::SameLine();
	DrawHelpMarker();

	std::vector<TimelineSlot> slots = ComputeTimelineLayout(stage);
	TrackGeometry geometry = BuildGeometry(stage, slots, pixelsPerSecond);

	DrawTrack(stage, slots, geometry, selectedIndex, pixelsPerSecond, draggingIndex, dragTime, actions, changed);
	DrawEntryTable(stage, slots, selectedIndex, actions, changed);

	// any structural edit invalidates the index a drag was tracking
	if(actions.Any()){
		draggingIndex = -1;
	}

	ApplyPendingActions(stage, slots, actions, selectedIndex, changed);

	ImGui::End();

	return changed;
}

}
