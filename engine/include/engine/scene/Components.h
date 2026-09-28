#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "engine/render/Texture.h"

namespace Engine {

struct Transform {
	glm::vec2 position{ 0.0f, 0.0f };
	glm::vec2 size{ 1.0f, 1.0f };
};

struct Sprite {
	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	std::shared_ptr<Texture> texture;
};

// A hitbox centered on the entity's Transform.position.
// Danmaku wants circles: a box clips corners the player never sees coming.
struct Collider {
	enum class Shape {
		Box,
		Circle,
	};

	Shape shape = Shape::Box;
	// size is what a box uses, radius is what a circle uses
	glm::vec2 size{ 1.0f, 1.0f };
	float radius = 0.5f;
};

}
