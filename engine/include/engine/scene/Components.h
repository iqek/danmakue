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

// An AABB hitbox, centered on the entity's Transform.position
struct Collider {
	glm::vec2 size{ 1.0f, 1.0f };
};

}
