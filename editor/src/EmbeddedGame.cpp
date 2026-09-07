#include "EmbeddedGame.h"

#include "engine/core/Log.h"
#include "engine/danmaku/EnemySystem.h"
#include "engine/danmaku/Player.h"
#include "engine/danmaku/PlayerSystem.h"
#include "engine/danmaku/stage/StageLoader.h"
#include "engine/platform/Input.h"
#include "engine/render/RenderSystem.h"
#include "engine/scene/Components.h"

#include <cmath>

namespace Editor {

EmbeddedGame::EmbeddedGame(const std::string& fontPath):
	framebuffer(kWidth, kHeight),
	renderer(kWidth, kHeight),
	font(fontPath, 16.0f)
{
	hud.AddElement({ Engine::HudStat::Lives, "LIVES: ", glm::vec2(20.0f, 20.0f) });
	hud.AddElement({ Engine::HudStat::Score, "SCORE: ", glm::vec2(20.0f, 44.0f) });
}

void EmbeddedGame::Start(const StageDefinition& stageDefinition, const std::string& stagePath){
	try{
		SaveStageDefinition(stageDefinition, stagePath);
	}
	catch(const std::exception& e){
		ENGINE_CORE_ERROR("Failed to save stage before playing {}: {}", stagePath, e.what());
		return;
	}

	registry.clear();
	gameOver = false;

	player = Engine::SpawnPlayer(registry, stagePath);
	stage = Engine::LoadStage(stagePath);

	glm::vec2 bounds(static_cast<float>(kWidth), static_cast<float>(kHeight));
	enemyBulletPool = std::make_unique<Engine::BulletPool>(1000, glm::vec2(0.0f), bounds);
	playerBulletPool = std::make_unique<Engine::BulletPool>(200, glm::vec2(0.0f), bounds);

	playing = true;
}

void EmbeddedGame::Stop(){
	playing = false;
	gameOver = false;
	registry.clear();
	enemyBulletPool.reset();
	playerBulletPool.reset();
}

void EmbeddedGame::Update(float deltaTime, bool inputAllowed){
	if(!playing || gameOver){
		return;
	}

	auto& transform = registry.get<Engine::Transform>(player);
	auto& playerData = registry.get<Engine::Player>(player);

	if(inputAllowed){
		if(Engine::Input::IsKeyPressed(Engine::Key::W) || Engine::Input::IsKeyPressed(Engine::Key::Up)){
			transform.position.y -= playerData.moveSpeed * deltaTime;
		}
		if(Engine::Input::IsKeyPressed(Engine::Key::S) || Engine::Input::IsKeyPressed(Engine::Key::Down)){
			transform.position.y += playerData.moveSpeed * deltaTime;
		}
		if(Engine::Input::IsKeyPressed(Engine::Key::A) || Engine::Input::IsKeyPressed(Engine::Key::Left)){
			transform.position.x -= playerData.moveSpeed * deltaTime;
		}
		if(Engine::Input::IsKeyPressed(Engine::Key::D) || Engine::Input::IsKeyPressed(Engine::Key::Right)){
			transform.position.x += playerData.moveSpeed * deltaTime;
		}
	}

	stage.Update(registry, deltaTime);
	Engine::UpdateEnemies(registry, *enemyBulletPool, deltaTime, transform.position);
	Engine::DespawnOffscreenEnemies(registry, glm::vec2(0.0f), glm::vec2(kWidth, kHeight));
	Engine::UpdatePlayerDamage(registry, player, *enemyBulletPool, deltaTime);

	bool firing = inputAllowed && Engine::Input::IsKeyPressed(Engine::Key::Space);
	Engine::UpdatePlayerWeapons(registry, player, *playerBulletPool, deltaTime, firing);
	Engine::UpdateEnemyHealth(registry, *playerBulletPool, enemyDamagePerHit, player, scorePerKill);

	enemyBulletPool->Update(deltaTime, transform.position);
	playerBulletPool->Update(deltaTime, transform.position);

	auto& playerSprite = registry.get<Engine::Sprite>(player);
	bool blinkOff = playerData.invincibleTimer > 0.0f && std::fmod(playerData.invincibleTimer, 0.2f) < 0.1f;
	playerSprite.color.a = blinkOff ? 0.3f : 1.0f;

	if(playerData.lives <= 0){
		gameOver = true;
		ENGINE_INFO("Game over. Final score: {}", playerData.score);
	}
}

void EmbeddedGame::Render(){
	framebuffer.Bind();
	renderer.Clear(glm::vec4(0.1f, 0.1f, 0.15f, 1.0f));

	if(playing){
		Engine::RenderSprites(registry, renderer);
		enemyBulletPool->Draw(renderer);
		playerBulletPool->Draw(renderer);
		hud.Draw(registry, player, renderer, font, glm::vec4(1.0f));

		if(gameOver){
			renderer.DrawText(glm::vec2(kWidth * 0.5f - 80.0f, kHeight * 0.5f), "GAME OVER", font, glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));
		}
	}

	framebuffer.Unbind();
}

}
