#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Engine {

class Renderer2D;
class Font;

enum class HudStat {
	Lives,
	Score,
};

struct HudElement {
	HudStat stat;
	std::string label;
	glm::vec2 position{ 0.0f, 0.0f };
};

// A small, swappable list of stat readouts drawn each frame - the default
// HUD is just one particular list of elements. Adding a new stat later means
// adding one HudStat value and one case in Draw(), not redesigning this class.
class Hud {
private:
	std::vector<HudElement> elements;

public:
	void AddElement(const HudElement& element);
	void Draw(entt::registry& registry, entt::entity player, const Renderer2D& renderer, const Font& font, const glm::vec4& color) const;
};

}
