#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Editor {

// One kind of component the editor can create and edit.
// Adding a kind means adding one entry to the table in ComponentTypes.cpp.
struct ComponentType {
	const char* type;
	const char* label;
	nlohmann::json (*makeDefault)();
	void (*draw)(nlohmann::json& component);
};

// The table itself, in the order Add Component offers them
const std::vector<ComponentType>& ComponentTypes();

// null when a stage names a component this build has never heard of
const ComponentType* FindComponentType(const std::string& type);

// What a freshly created enemy starts with
nlohmann::json DefaultEnemyComponents();

}
