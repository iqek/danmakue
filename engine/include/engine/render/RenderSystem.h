#pragma once

#include <entt/entt.hpp>

namespace Engine {

class Renderer2D;

// draws every entity that has both a Transform and a Sprite component.
void RenderSprites(entt::registry& registry, const Renderer2D& renderer);

}
