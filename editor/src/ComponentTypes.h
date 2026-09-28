#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Editor {

// Which entities a kind makes sense on, so Add Component only offers what fits
enum class ComponentOwner {
	Enemy,
	Player,
	Both,
};

// One kind of component the editor can create and edit.
// Adding a kind means adding one entry to the table in ComponentTypes.cpp.
struct ComponentType {
	const char* type;
	const char* label;
	ComponentOwner owner;
	nlohmann::json (*makeDefault)();
	void (*draw)(nlohmann::json& component);
};

// The table itself, in the order Add Component offers them
const std::vector<ComponentType>& ComponentTypes();

// null when a stage names a component this build has never heard of
const ComponentType* FindComponentType(const std::string& type);

// owner is Enemy or Player here, never Both
bool ComponentAppliesTo(const ComponentType& type, ComponentOwner owner);

// What a freshly created enemy or player starts with
nlohmann::json DefaultEnemyComponents();
nlohmann::json DefaultPlayerComponents();

// The add and remove list both the enemy and the player inspectors show
void DrawComponentList(nlohmann::json& components, ComponentOwner owner);

}
