#include "engine/render/RenderSystem.h"
#include "engine/scene/Components.h"
#include "engine/render/Renderer2D.h"

namespace Engine {

void RenderSprites(entt::registry& registry, const Renderer2D& renderer){
	auto view = registry.view<Transform, Sprite>();
	for(auto entity : view){
		auto [transform, sprite] = view.get<Transform, Sprite>(entity);
		if(sprite.texture){
			renderer.DrawQuad(transform.position, transform.size, *sprite.texture, sprite.color);
		} else {
			renderer.DrawQuad(transform.position, transform.size, sprite.color);
		}
	}
}

}
