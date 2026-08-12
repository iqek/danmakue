#include "engine/danmaku/AimedShotEmitter.h"
#include "engine/danmaku/BulletPool.h"
#include "engine/danmaku/Enemy.h"
#include "engine/danmaku/EnemySystem.h"
#include "engine/danmaku/LinearMovement.h"
#include "engine/danmaku/Player.h"
#include "engine/danmaku/PlayerSystem.h"
#include "engine/danmaku/RadialBurstEmitter.h"
#include "engine/danmaku/RandomScatterEmitter.h"
#include "engine/danmaku/SpiralEmitter.h"
#include "engine/danmaku/StraightShotEmitter.h"
#include "engine/danmaku/WaypointMovement.h"
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
#include <vector>

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

	// Enters from the top, moves through three spots (pausing to fire at
	// each), and fires two patterns at once throughout.
	auto enemyA = registry.create();
	registry.emplace<Engine::Transform>(enemyA, glm::vec2(400.0f, -50.0f), glm::vec2(40.0f, 40.0f));
	registry.emplace<Engine::Sprite>(enemyA, glm::vec4(0.75f, 0.55f, 1.0f, 1.0f));
	registry.emplace<Engine::Collider>(enemyA, glm::vec2(36.0f, 36.0f));
	Engine::Enemy enemyAData;
	enemyAData.health = 50;
	enemyAData.maxHealth = 50;
	std::vector<Engine::Waypoint> enemyAPath = {
		{ glm::vec2(400.0f, 150.0f), 2.0f },
		{ glm::vec2(700.0f, 250.0f), 2.0f },
		{ glm::vec2(300.0f, 300.0f), 2.0f },
	};
	enemyAData.movement = std::make_unique<Engine::WaypointMovement>(enemyAPath, 150.0f);
	enemyAData.emitters.push_back(std::make_unique<Engine::RadialBurstEmitter>(16, 120.0f, 2.0f, glm::vec4(0.75f, 0.55f, 1.0f, 1.0f)));
	enemyAData.emitters.push_back(std::make_unique<Engine::SpiralEmitter>(150.0f, 0.04f, 2.5f, 2, glm::vec4(0.5f, 0.95f, 0.85f, 1.0f)));
	registry.emplace<Engine::Enemy>(enemyA, std::move(enemyAData));

	// Drifts sideways forever, tracking the player while it goes.
	auto enemyB = registry.create();
	registry.emplace<Engine::Transform>(enemyB, glm::vec2(-50.0f, 500.0f), glm::vec2(40.0f, 40.0f));
	registry.emplace<Engine::Sprite>(enemyB, glm::vec4(1.0f, 0.85f, 0.5f, 1.0f));
	registry.emplace<Engine::Collider>(enemyB, glm::vec2(36.0f, 36.0f));
	Engine::Enemy enemyBData;
	enemyBData.health = 50;
	enemyBData.maxHealth = 50;
	enemyBData.movement = std::make_unique<Engine::LinearMovement>(glm::vec2(150.0f, 0.0f));
	enemyBData.emitters.push_back(std::make_unique<Engine::AimedShotEmitter>(200.0f, 0.6f, glm::vec4(1.0f, 0.85f, 0.5f, 1.0f)));
	enemyBData.emitters.push_back(std::make_unique<Engine::RandomScatterEmitter>(60.0f, 180.0f, 0.05f, glm::vec4(1.0f, 0.65f, 0.85f, 1.0f)));
	registry.emplace<Engine::Enemy>(enemyB, std::move(enemyBData));

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

			Engine::UpdateEnemies(registry, enemyBulletPool, dt, transform.position);
			Engine::UpdatePlayerDamage(registry, player, enemyBulletPool, dt);

			bool firing = Engine::Input::IsKeyPressed(Engine::Key::Space);
			Engine::UpdatePlayerWeapons(registry, player, playerBulletPool, dt, firing);
			Engine::UpdateEnemyHealth(registry, playerBulletPool, enemyDamagePerHit, player, scorePerKill);

			enemyBulletPool.Update(dt);
			playerBulletPool.Update(dt);

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
