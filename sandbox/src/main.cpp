#include "engine/danmaku/BulletPool.h"
#include "engine/danmaku/EnemySystem.h"
#include "engine/danmaku/Player.h"
#include "engine/danmaku/PlayerSystem.h"
#include "engine/danmaku/emitters/StraightShotEmitter.h"
#include "engine/danmaku/stage/Stage.h"
#include "engine/danmaku/stage/StageLoader.h"
#include "engine/scene/CollisionSystem.h"
#include "engine/scene/Components.h"
#include "engine/platform/Input.h"
#include "engine/core/Log.h"
#include "engine/render/Font.h"
#include "engine/render/Renderer2D.h"
#include "engine/render/RenderSystem.h"
#include "engine/render/TextureLibrary.h"
#include "engine/core/Time.h"
#include "engine/platform/Window.h"
#include "engine/ui/Hud.h"

#include <cmath>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <memory>

int main(){
	Engine::Log::Init();
	ENGINE_INFO("Sandbox started");

	Engine::Window window;
	Engine::Input::Init(window.GetHandle());
	Engine::Renderer2D renderer(window.GetWidth(), window.GetHeight());
	Engine::Font hudFont("assets/PressStart2P-Regular.ttf", 16.0f);

	Engine::Hud hud;
	hud.AddElement({ Engine::HudStat::Lives, "LIVES: ", glm::vec2(20.0f, 20.0f) });
	hud.AddElement({ Engine::HudStat::Score, "SCORE: ", glm::vec2(20.0f, 44.0f) });

	entt::registry registry;

	auto player = registry.create();
	registry.emplace<Engine::Transform>(player, glm::vec2(640.0f, 360.0f), glm::vec2(80.0f, 80.0f));
	registry.emplace<Engine::Sprite>(player, glm::vec4(1.0f, 0.55f, 0.65f, 1.0f));
	registry.emplace<Engine::Collider>(player, glm::vec2(30.0f, 30.0f));
	Engine::Player initialPlayerData;
	initialPlayerData.weapons.push_back(std::make_unique<Engine::StraightShotEmitter>(glm::vec2(0.0f, -1.0f), 500.0f, 0.12f, glm::vec4(1.0f, 0.55f, 0.65f, 1.0f)));
	registry.emplace<Engine::Player>(player, std::move(initialPlayerData));

	auto patchouliEntity = registry.create();
	registry.emplace<Engine::Transform>(patchouliEntity, glm::vec2(150.0f, 150.0f), glm::vec2(128.0f, 128.0f));
	registry.emplace<Engine::Sprite>(patchouliEntity, glm::vec4(1.0f), Engine::TextureLibrary::Get("assets/know.png"));
	registry.emplace<Engine::Collider>(patchouliEntity, glm::vec2(110.0f, 110.0f));

	Engine::Stage stage = Engine::LoadStage("assets/stage1.json");

	Engine::BulletPool enemyBulletPool(1000, glm::vec2(0.0f, 0.0f), glm::vec2(window.GetWidth(), window.GetHeight()));
	Engine::BulletPool playerBulletPool(200, glm::vec2(0.0f, 0.0f), glm::vec2(window.GetWidth(), window.GetHeight()));

	float speed = 300.0f;
	int enemyDamagePerHit = 10;
	int scorePerKill = 100;
	bool gameOver = false;

	while(!window.ShouldClose()){
		Engine::Time::Update();

		if(Engine::Input::IsKeyPressed(Engine::Key::Escape)){
			break;
		}

		float dt = Engine::Time::GetDeltaTime();

		if(!gameOver){
			auto& transform = registry.get<Engine::Transform>(player);
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

			bool touching = Engine::CheckCollision(registry, player, patchouliEntity);
			auto& patchouliSprite = registry.get<Engine::Sprite>(patchouliEntity);
			patchouliSprite.color = touching ? glm::vec4(1.0f, 0.45f, 0.5f, 1.0f) : glm::vec4(1.0f);

			stage.Update(registry, dt);
			Engine::UpdateEnemies(registry, enemyBulletPool, dt, transform.position);
			Engine::DespawnOffscreenEnemies(registry, glm::vec2(0.0f, 0.0f), glm::vec2(window.GetWidth(), window.GetHeight()));
			Engine::UpdatePlayerDamage(registry, player, enemyBulletPool, dt);

			bool firing = Engine::Input::IsKeyPressed(Engine::Key::Space);
			Engine::UpdatePlayerWeapons(registry, player, playerBulletPool, dt, firing);
			Engine::UpdateEnemyHealth(registry, playerBulletPool, enemyDamagePerHit, player, scorePerKill);

			enemyBulletPool.Update(dt, transform.position);
			playerBulletPool.Update(dt, transform.position);

			auto& playerData = registry.get<Engine::Player>(player);
			auto& playerSprite = registry.get<Engine::Sprite>(player);
			bool blinkOff = playerData.invincibleTimer > 0.0f && std::fmod(playerData.invincibleTimer, 0.2f) < 0.1f;
			playerSprite.color.a = blinkOff ? 0.3f : 1.0f;

			if(playerData.lives <= 0){
				gameOver = true;
				ENGINE_INFO("Game over. Final score: {}", playerData.score);
			}
		}

		renderer.Clear({ 0.1f, 0.1f, 0.15f, 1.0f });
		Engine::RenderSprites(registry, renderer);
		enemyBulletPool.Draw(renderer);
		playerBulletPool.Draw(renderer);
		hud.Draw(registry, player, renderer, hudFont, glm::vec4(1.0f));

		if(gameOver){
			renderer.DrawText(glm::vec2(520.0f, 340.0f), "GAME OVER", hudFont, glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));
		}

		window.OnUpdate();
	}

	return 0;
}
