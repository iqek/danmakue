#include "engine/ui/Hud.h"
#include "engine/danmaku/Player.h"
#include "engine/render/Renderer2D.h"

namespace Engine {

void Hud::AddElement(const HudElement& element){
	elements.push_back(element);
}

void Hud::Draw(entt::registry& registry, entt::entity player, const Renderer2D& renderer, const Font& font, const glm::vec4& color) const{
	auto& playerData = registry.get<Player>(player);

	for(const auto& element : elements){
		int value = 0;
		switch(element.stat){
			case HudStat::Lives:
				value = playerData.lives;
				break;
			case HudStat::Score:
				value = playerData.score;
				break;
		}

		std::string text = element.label + std::to_string(value);
		renderer.DrawText(element.position, text, font, color);
	}
}

}
