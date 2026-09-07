#pragma once

#include "JsonFloat.h"

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace Editor {

inline glm::vec2 GetVec2(const nlohmann::json& json, const char* key, const glm::vec2& fallback){
	if(!json.contains(key)){
		return fallback;
	}
	const nlohmann::json& array = json.at(key);
	return glm::vec2(array.at(0).get<float>(), array.at(1).get<float>());
}

inline void SetVec2(nlohmann::json& json, const char* key, const glm::vec2& v){
	json[key] = nlohmann::json::array({ CleanFloat(v.x), CleanFloat(v.y) });
}

inline glm::vec4 GetVec4(const nlohmann::json& json, const char* key, const glm::vec4& fallback){
	if(!json.contains(key)){
		return fallback;
	}
	const nlohmann::json& array = json.at(key);
	return glm::vec4(array.at(0).get<float>(), array.at(1).get<float>(), array.at(2).get<float>(), array.at(3).get<float>());
}

inline void SetVec4(nlohmann::json& json, const char* key, const glm::vec4& v){
	json[key] = nlohmann::json::array({ CleanFloat(v.x), CleanFloat(v.y), CleanFloat(v.z), CleanFloat(v.w) });
}

inline void SetFloat(nlohmann::json& json, const char* key, float value){
	json[key] = CleanFloat(value);
}

}
