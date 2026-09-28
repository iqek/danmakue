#include "ComponentTypes.h"
#include "EmitterListEditor.h"
#include "JsonFieldHelpers.h"
#include "StageDefinition.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace Editor {

namespace {

using Json = nlohmann::json;

void DrawSprite(Json& component){
	glm::vec4 color = GetVec4(component, "color", glm::vec4(1.0f));
	if(ImGui::ColorEdit4("Color", &color.x)){
		SetVec4(component, "color", color);
	}

	std::string texture = component.value("texture", std::string());
	if(ImGui::InputTextWithHint("Texture", "know.png", &texture)){
		if(texture.empty()){
			component.erase("texture");
		}
		else{
			component["texture"] = texture;
		}
	}
	ImGui::TextDisabled("Path inside the assets folder; leave it blank for a plain colour");
}

void DrawCollider(Json& component){
	glm::vec2 size = GetVec2(component, "size", glm::vec2(32.0f));
	if(ImGui::DragFloat2("Size", &size.x)){
		SetVec2(component, "size", size);
	}
	ImGui::TextDisabled("The box that counts as a hit, centred on the enemy");
}

void DrawHealth(Json& component){
	int hp = component.value("hp", 1);
	if(ImGui::DragInt("HP", &hp, 1.0f, 1, 10000)){
		component["hp"] = hp;
	}
}

void DrawMovementPattern(Json& pattern){
	std::string type = pattern.value("type", std::string("linear"));

	const char* typeNames[] = { "linear", "waypoint" };
	int typeIndex = (type == "waypoint") ? 1 : 0;

	if(ImGui::Combo("Type##movement", &typeIndex, typeNames, IM_ARRAYSIZE(typeNames))){
		std::string newType = typeNames[typeIndex];
		if(newType == "linear"){
			pattern = Json{ { "type", "linear" }, { "velocity", Json::array({ 0.0, 0.0 }) } };
		}
		else{
			pattern = Json{ { "type", "waypoint" }, { "speed", 100.0 }, { "waypoints", Json::array() } };
		}
		return;
	}

	if(type == "linear"){
		glm::vec2 velocity = GetVec2(pattern, "velocity", glm::vec2(0.0f));
		if(ImGui::DragFloat2("Velocity", &velocity.x)){
			SetVec2(pattern, "velocity", velocity);
		}
		return;
	}

	float speed = pattern.value("speed", 100.0f);
	if(ImGui::DragFloat("Speed", &speed, 1.0f, 0.0f, 2000.0f)){
		SetFloat(pattern, "speed", speed);
	}

	ImGui::TextDisabled("Offsets are from the spawn point, so the path travels with it");

	if(!pattern.contains("waypoints") || !pattern.at("waypoints").is_array()){
		pattern["waypoints"] = Json::array();
	}
	Json& waypoints = pattern["waypoints"];

	int removeIndex = -1;
	for(int i = 0; i < static_cast<int>(waypoints.size()); i++){
		ImGui::PushID(i);
		Json& point = waypoints[i];

		glm::vec2 offset = GetVec2(point, "offset", glm::vec2(0.0f));
		if(ImGui::DragFloat2("Offset", &offset.x)){
			SetVec2(point, "offset", offset);
		}

		float waitTime = point.value("waitTime", 0.0f);
		if(ImGui::DragFloat("Wait Time", &waitTime, 0.1f, 0.0f, 60.0f)){
			SetFloat(point, "waitTime", waitTime);
		}

		if(ImGui::Button("Remove Waypoint")){
			removeIndex = i;
		}

		ImGui::Separator();
		ImGui::PopID();
	}

	if(removeIndex >= 0){
		RemoveWaypoint(pattern, removeIndex);
	}

	if(ImGui::Button("Add Waypoint")){
		InsertWaypointAfter(pattern, WaypointCount(pattern) - 1);
	}
}

void DrawMovement(Json& component){
	if(!component.contains("pattern")){
		component["pattern"] = Json{ { "type", "linear" }, { "velocity", Json::array({ 0.0, 0.0 }) } };
	}
	DrawMovementPattern(component["pattern"]);
}

void DrawEmitters(Json& component){
	if(!component.contains("list") || !component.at("list").is_array()){
		component["list"] = Json::array();
	}
	DrawEmitterList(component["list"]);
}

Json SpriteDefault(){
	return Json{ { "type", "sprite" }, { "color", Json::array({ 1.0, 1.0, 1.0, 1.0 }) } };
}

Json ColliderDefault(){
	return Json{ { "type", "collider" }, { "size", Json::array({ 32.0, 32.0 }) } };
}

Json HealthDefault(){
	return Json{ { "type", "health" }, { "hp", 1 } };
}

Json MovementDefault(){
	// a zero velocity keeps the JSON valid, so a fresh enemy can already spawn
	return Json{ { "type", "movement" }, { "pattern", Json{ { "type", "linear" }, { "velocity", Json::array({ 0.0, 0.0 }) } } } };
}

Json EmittersDefault(){
	return Json{ { "type", "emitters" }, { "list", Json::array() } };
}

}

const std::vector<ComponentType>& ComponentTypes(){
	static const std::vector<ComponentType> types = {
		{ "sprite", "Sprite", SpriteDefault, DrawSprite },
		{ "collider", "Collider", ColliderDefault, DrawCollider },
		{ "health", "Health", HealthDefault, DrawHealth },
		{ "movement", "Movement", MovementDefault, DrawMovement },
		{ "emitters", "Emitters", EmittersDefault, DrawEmitters },
	};
	return types;
}

const ComponentType* FindComponentType(const std::string& type){
	for(const auto& entry : ComponentTypes()){
		if(type == entry.type){
			return &entry;
		}
	}
	return nullptr;
}

Json DefaultEnemyComponents(){
	Json components = Json::array();
	for(const auto& entry : ComponentTypes()){
		components.push_back(entry.makeDefault());
	}
	return components;
}

}
