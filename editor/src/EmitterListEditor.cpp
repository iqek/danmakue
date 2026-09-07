#include "EmitterListEditor.h"
#include "JsonFieldHelpers.h"

#include <imgui.h>
#include <string>

namespace Editor {

namespace {

using Json = nlohmann::json;

// default shape for a freshly-added or freshly-retyped emitter, matching the
// fields StageLoader::CreateEmitter expects for each type
Json DefaultEmitter(const std::string& type){
	Json defaultColor = Json::array({ 1.0, 1.0, 1.0, 1.0 });

	if(type == "spiral"){
		return Json{ { "type", "spiral" }, { "bulletSpeed", 120.0 }, { "spawnInterval", 0.05 }, { "angularVelocity", 2.0 }, { "arms", 1 }, { "color", defaultColor } };
	}
	if(type == "aimedShot"){
		return Json{ { "type", "aimedShot" }, { "bulletSpeed", 200.0 }, { "interval", 0.5 }, { "color", defaultColor }, { "homingStrength", 0.0 } };
	}
	if(type == "randomScatter"){
		return Json{ { "type", "randomScatter" }, { "minSpeed", 60.0 }, { "maxSpeed", 180.0 }, { "spawnInterval", 0.05 }, { "color", defaultColor }, { "outwardAcceleration", 0.0 } };
	}
	if(type == "straightShot"){
		return Json{ { "type", "straightShot" }, { "direction", Json::array({ 0.0, -1.0 }) }, { "bulletSpeed", 300.0 }, { "interval", 0.2 }, { "color", defaultColor } };
	}
	// radialBurst, and the fallback for anything unrecognized
	return Json{ { "type", "radialBurst" }, { "bulletCount", 16 }, { "bulletSpeed", 120.0 }, { "interval", 1.0 }, { "color", defaultColor } };
}

void DrawEmitterFields(Json& emitter, const std::string& type){
	if(type == "radialBurst"){
		int bulletCount = emitter.value("bulletCount", 16);
		if(ImGui::DragInt("Bullet Count", &bulletCount, 1, 1, 360)){
			emitter["bulletCount"] = bulletCount;
		}
		float bulletSpeed = emitter.value("bulletSpeed", 120.0f);
		if(ImGui::DragFloat("Bullet Speed", &bulletSpeed, 1.0f, 0.0f, 2000.0f)){
			SetFloat(emitter, "bulletSpeed", bulletSpeed);
		}
		float interval = emitter.value("interval", 1.0f);
		if(ImGui::DragFloat("Interval", &interval, 0.05f, 0.01f, 30.0f)){
			SetFloat(emitter, "interval", interval);
		}
	}
	else if(type == "spiral"){
		float bulletSpeed = emitter.value("bulletSpeed", 120.0f);
		if(ImGui::DragFloat("Bullet Speed", &bulletSpeed, 1.0f, 0.0f, 2000.0f)){
			SetFloat(emitter, "bulletSpeed", bulletSpeed);
		}
		float spawnInterval = emitter.value("spawnInterval", 0.05f);
		if(ImGui::DragFloat("Spawn Interval", &spawnInterval, 0.01f, 0.01f, 5.0f)){
			SetFloat(emitter, "spawnInterval", spawnInterval);
		}
		float angularVelocity = emitter.value("angularVelocity", 2.0f);
		if(ImGui::DragFloat("Angular Velocity", &angularVelocity, 0.05f, -20.0f, 20.0f)){
			SetFloat(emitter, "angularVelocity", angularVelocity);
		}
		int arms = emitter.value("arms", 1);
		if(ImGui::DragInt("Arms", &arms, 1, 1, 12)){
			emitter["arms"] = arms;
		}
	}
	else if(type == "aimedShot"){
		float bulletSpeed = emitter.value("bulletSpeed", 200.0f);
		if(ImGui::DragFloat("Bullet Speed", &bulletSpeed, 1.0f, 0.0f, 2000.0f)){
			SetFloat(emitter, "bulletSpeed", bulletSpeed);
		}
		float interval = emitter.value("interval", 0.5f);
		if(ImGui::DragFloat("Interval", &interval, 0.05f, 0.01f, 30.0f)){
			SetFloat(emitter, "interval", interval);
		}
		float homingStrength = emitter.value("homingStrength", 0.0f);
		if(ImGui::DragFloat("Homing Strength", &homingStrength, 0.05f, 0.0f, 10.0f)){
			SetFloat(emitter, "homingStrength", homingStrength);
		}
	}
	else if(type == "randomScatter"){
		float minSpeed = emitter.value("minSpeed", 60.0f);
		if(ImGui::DragFloat("Min Speed", &minSpeed, 1.0f, 0.0f, 2000.0f)){
			SetFloat(emitter, "minSpeed", minSpeed);
		}
		float maxSpeed = emitter.value("maxSpeed", 180.0f);
		if(ImGui::DragFloat("Max Speed", &maxSpeed, 1.0f, 0.0f, 2000.0f)){
			SetFloat(emitter, "maxSpeed", maxSpeed);
		}
		float spawnInterval = emitter.value("spawnInterval", 0.05f);
		if(ImGui::DragFloat("Spawn Interval", &spawnInterval, 0.01f, 0.01f, 5.0f)){
			SetFloat(emitter, "spawnInterval", spawnInterval);
		}
		float outwardAcceleration = emitter.value("outwardAcceleration", 0.0f);
		if(ImGui::DragFloat("Outward Acceleration", &outwardAcceleration, 1.0f, -500.0f, 500.0f)){
			SetFloat(emitter, "outwardAcceleration", outwardAcceleration);
		}
	}
	else if(type == "straightShot"){
		glm::vec2 direction = GetVec2(emitter, "direction", glm::vec2(0.0f, -1.0f));
		if(ImGui::DragFloat2("Direction", &direction.x, 0.05f)){
			SetVec2(emitter, "direction", direction);
		}
		float bulletSpeed = emitter.value("bulletSpeed", 300.0f);
		if(ImGui::DragFloat("Bullet Speed", &bulletSpeed, 1.0f, 0.0f, 2000.0f)){
			SetFloat(emitter, "bulletSpeed", bulletSpeed);
		}
		float interval = emitter.value("interval", 0.2f);
		if(ImGui::DragFloat("Interval", &interval, 0.05f, 0.01f, 30.0f)){
			SetFloat(emitter, "interval", interval);
		}
	}

	glm::vec4 color = GetVec4(emitter, "color", glm::vec4(1.0f));
	if(ImGui::ColorEdit4("Color", &color.x)){
		SetVec4(emitter, "color", color);
	}
}

}

void DrawEmitterList(Json& emitters){
	if(!emitters.is_array()){
		emitters = Json::array();
	}

	const char* typeNames[] = { "radialBurst", "spiral", "aimedShot", "randomScatter", "straightShot" };

	int removeIndex = -1;
	for(int i = 0; i < static_cast<int>(emitters.size()); i++){
		ImGui::PushID(i);
		Json& emitter = emitters[i];
		std::string type = emitter.value("type", std::string("radialBurst"));

		int typeIndex = 0;
		for(int t = 0; t < IM_ARRAYSIZE(typeNames); t++){
			if(type == typeNames[t]){
				typeIndex = t;
				break;
			}
		}

		if(ImGui::CollapsingHeader((type + "##header").c_str(), ImGuiTreeNodeFlags_DefaultOpen)){
			if(ImGui::Combo("Type##emitter", &typeIndex, typeNames, IM_ARRAYSIZE(typeNames))){
				std::string newType = typeNames[typeIndex];
				if(newType != type){
					emitter = DefaultEmitter(newType);
				}
			}
			else{
				DrawEmitterFields(emitter, type);
			}

			if(ImGui::Button("Remove Emitter")){
				removeIndex = i;
			}
		}

		ImGui::PopID();
	}

	if(removeIndex >= 0){
		emitters.erase(emitters.begin() + removeIndex);
	}

	if(ImGui::Button("Add Emitter")){
		emitters.push_back(DefaultEmitter("radialBurst"));
	}
}

}
