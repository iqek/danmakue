#include "engine/danmaku/AimedShotEmitter.h"
#include "engine/danmaku/BulletEmitter.h"
#include "engine/danmaku/BulletPool.h"
#include "engine/danmaku/RadialBurstEmitter.h"
#include "engine/danmaku/RandomScatterEmitter.h"
#include "engine/danmaku/SpiralEmitter.h"
#include "engine/scene/CollisionSystem.h"
#include "engine/scene/Components.h"
#include "engine/platform/Input.h"
#include "engine/core/Log.h"
#include "engine/render/Renderer2D.h"
#include "engine/render/RenderSystem.h"
#include "engine/render/TextureLibrary.h"
#include "engine/core/Time.h"
#include "engine/platform/Window.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

int main(){
	Engine::Log::Init();
	ENGINE_INFO("Sandbox started");

	Engine::Window window;
	Engine::Input::Init(window.GetHandle());
	Engine::Renderer2D renderer(window.GetWidth(), window.GetHeight());

	entt::registry registry;

	auto player = registry.create();
	registry.emplace<Engine::Transform>(player, glm::vec2(640.0f, 360.0f), glm::vec2(80.0f, 80.0f));
	registry.emplace<Engine::Sprite>(player, glm::vec4(1.0f, 0.55f, 0.65f, 1.0f));
	registry.emplace<Engine::Collider>(player, glm::vec2(30.0f, 30.0f));

	auto patchouliEntity = registry.create();
	registry.emplace<Engine::Transform>(patchouliEntity, glm::vec2(150.0f, 150.0f), glm::vec2(128.0f, 128.0f));
	registry.emplace<Engine::Sprite>(patchouliEntity, glm::vec4(1.0f), Engine::TextureLibrary::Get("assets/know.png"));
	registry.emplace<Engine::Collider>(patchouliEntity, glm::vec2(110.0f, 110.0f));

	Engine::BulletPool bulletPool(1000, glm::vec2(0.0f, 0.0f), glm::vec2(window.GetWidth(), window.GetHeight()));

	std::vector<std::unique_ptr<Engine::BulletEmitter>> emitters;
	emitters.push_back(std::make_unique<Engine::RadialBurstEmitter>(glm::vec2(320.0f, 200.0f), 24, 150.0f, 1.5f, glm::vec4(0.75f, 0.55f, 1.0f, 1.0f)));
	emitters.push_back(std::make_unique<Engine::SpiralEmitter>(glm::vec2(960.0f, 200.0f), 180.0f, 0.03f, 3.0f, 3, glm::vec4(0.5f, 0.95f, 0.85f, 1.0f)));
	emitters.push_back(std::make_unique<Engine::AimedShotEmitter>(glm::vec2(640.0f, 650.0f), 250.0f, 0.5f, glm::vec4(1.0f, 0.85f, 0.5f, 1.0f)));
	emitters.push_back(std::make_unique<Engine::RandomScatterEmitter>(glm::vec2(640.0f, 100.0f), 80.0f, 250.0f, 0.02f, glm::vec4(1.0f, 0.65f, 0.85f, 1.0f)));

	float speed = 300.0f;

	while(!window.ShouldClose()){
		Engine::Time::Update();

		if(Engine::Input::IsKeyPressed(Engine::Key::Escape)){
			break;
		}

		auto& transform = registry.get<Engine::Transform>(player);
		float dt = Engine::Time::GetDeltaTime();
		if(Engine::Input::IsKeyPressed(Engine::Key::W) || Engine::Input::IsKeyPressed(Engine::Key::Up)){
			transform.position.y -= speed * dt;
		}
		if(Engine::Input::IsKeyPressed(Engine::Key::S) || Engine::Input::IsKeyPressed(Engine::Key::Down)){
			transform.position.y += speed * dt;
		}
		if(Engine::Input::IsKeyPressed(Engine::Key::A) || Engine::Input::IsKeyPressed(Engine::Key::Left)){
			transform.position.x -= speed * dt;
		}
		if(Engine::Input::IsKeyPressed(Engine::Key::D) || Engine::Input::IsKeyPressed(Engine::Key::Right)){
			transform.position.x += speed * dt;
		}

		bool touching = !Engine::FindCollisions(registry).empty();
		auto& patchouliSprite = registry.get<Engine::Sprite>(patchouliEntity);
		patchouliSprite.color = touching ? glm::vec4(1.0f, 0.45f, 0.5f, 1.0f) : glm::vec4(1.0f);

		for(auto& emitter : emitters){
			emitter->Update(dt, bulletPool, transform.position);
		}
		bulletPool.Update(dt);

		renderer.Clear({ 0.1f, 0.1f, 0.15f, 1.0f });
		Engine::RenderSprites(registry, renderer);
		bulletPool.Draw(renderer);

		window.OnUpdate();
	}

	return 0;
}
