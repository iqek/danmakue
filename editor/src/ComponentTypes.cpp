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
	if(ImGui::InputTextWithHint("Texture", "drag an image here", &texture)){
		if(texture.empty()){
			component.erase("texture");
		}
		else{
			component["texture"] = texture;
		}
	}

	// the field takes drops straight from the Assets panel, so the path is never typed wrong
	if(ImGui::BeginDragDropTarget()){
		if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE")){
			component["texture"] = static_cast<const char*>(payload->Data);
		}
		ImGui::EndDragDropTarget();
	}

	if(component.contains("texture")){
		ImGui::SameLine();
		if(ImGui::SmallButton("Clear")){
			component.erase("texture");
		}
	}

	ImGui::TextDisabled("Drag one from Assets, or type a path inside the assets folder");
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

void DrawPlayerSettings(Json& component){
	int lives = component.value("lives", 3);
	if(ImGui::DragInt("Lives", &lives, 1.0f, 1, 99)){
		component["lives"] = lives;
	}

	float moveSpeed = component.value("moveSpeed", 300.0f);
	if(ImGui::DragFloat("Move Speed", &moveSpeed, 1.0f, 0.0f, 2000.0f)){
		SetFloat(component, "moveSpeed", moveSpeed);
	}
}

void DrawWeapons(Json& component){
	if(!component.contains("list") || !component.at("list").is_array()){
		component["list"] = Json::array();
	}
	DrawEmitterList(component["list"]);
	ImGui::TextDisabled("An empty list is allowed; the player simply cannot shoot");
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

Json PlayerSettingsDefault(){
	return Json{ { "type", "player" }, { "lives", 3 }, { "moveSpeed", 300.0 } };
}

Json WeaponsDefault(){
	// a plain forward shot, so a brand new player can already do something
	return Json{ { "type", "weapons" }, { "list", Json::array({ Json{
		{ "type", "straightShot" },
		{ "direction", Json::array({ 0.0, -1.0 }) },
		{ "bulletSpeed", 500.0 },
		{ "interval", 0.12 },
		{ "color", Json::array({ 1.0, 1.0, 1.0, 1.0 }) }
	} }) } };
}

}

const std::vector<ComponentType>& ComponentTypes(){
	static const std::vector<ComponentType> types = {
		{ "sprite", "Sprite", ComponentOwner::Both, SpriteDefault, DrawSprite },
		{ "collider", "Collider", ComponentOwner::Both, ColliderDefault, DrawCollider },
		{ "health", "Health", ComponentOwner::Enemy, HealthDefault, DrawHealth },
		{ "movement", "Movement", ComponentOwner::Enemy, MovementDefault, DrawMovement },
		{ "emitters", "Emitters", ComponentOwner::Enemy, EmittersDefault, DrawEmitters },
		{ "player", "Player", ComponentOwner::Player, PlayerSettingsDefault, DrawPlayerSettings },
		{ "weapons", "Weapons", ComponentOwner::Player, WeaponsDefault, DrawWeapons },
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

bool ComponentAppliesTo(const ComponentType& type, ComponentOwner owner){
	return type.owner == ComponentOwner::Both || type.owner == owner;
}

namespace {

Json DefaultComponentsFor(ComponentOwner owner){
	Json components = Json::array();
	for(const auto& entry : ComponentTypes()){
		if(ComponentAppliesTo(entry, owner)){
			components.push_back(entry.makeDefault());
		}
	}
	return components;
}

}

Json DefaultEnemyComponents(){
	return DefaultComponentsFor(ComponentOwner::Enemy);
}

Json DefaultPlayerComponents(){
	return DefaultComponentsFor(ComponentOwner::Player);
}

void DrawComponentList(Json& components, ComponentOwner owner){
	if(!components.is_array()){
		components = Json::array();
	}

	int removeIndex = -1;
	for(int i = 0; i < static_cast<int>(components.size()); i++){
		Json& component = components[i];
		std::string type = component.value("type", std::string());
		const ComponentType* kind = FindComponentType(type);

		ImGui::PushID(i);

		// the header's close button is the remove, the way Unity does it
		bool keep = true;
		bool open = ImGui::CollapsingHeader(kind != nullptr ? kind->label : type.c_str(), &keep, ImGuiTreeNodeFlags_DefaultOpen);
		if(!keep){
			removeIndex = i;
		}

		if(open){
			ImGui::Indent();
			if(kind != nullptr){
				kind->draw(component);
			}
			else{
				ImGui::TextDisabled("This build does not know this component");
			}
			ImGui::Unindent();
		}

		ImGui::PopID();
	}

	if(removeIndex >= 0){
		components.erase(components.begin() + removeIndex);
	}

	ImGui::Spacing();
	if(ImGui::Button("Add Component")){
		ImGui::OpenPopup("addComponent");
	}

	if(ImGui::BeginPopup("addComponent")){
		bool anyLeft = false;
		for(const auto& kind : ComponentTypes()){
			if(!ComponentAppliesTo(kind, owner) || FindComponent(components, kind.type) != nullptr){
				continue;
			}
			anyLeft = true;
			if(ImGui::MenuItem(kind.label)){
				components.push_back(kind.makeDefault());
			}
		}
		if(!anyLeft){
			ImGui::TextDisabled("Nothing left to add");
		}
		ImGui::EndPopup();
	}
}

}
