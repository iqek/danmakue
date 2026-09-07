#pragma once

#include "StageDefinition.h"

#include "engine/danmaku/BulletPool.h"
#include "engine/danmaku/stage/Stage.h"
#include "engine/render/Font.h"
#include "engine/render/Framebuffer.h"
#include "engine/render/Renderer2D.h"
#include "engine/ui/Hud.h"

#include <entt/entt.hpp>
#include <memory>
#include <string>

namespace Editor {

// Runs the actual game simulation in-process and renders it into an offscreen
// texture, so Play can preview inside a docked panel instead of a separate window
class EmbeddedGame {
private:
	Engine::Framebuffer framebuffer;
	Engine::Renderer2D renderer;
	Engine::Font font;
	Engine::Hud hud;

	entt::registry registry;
	entt::entity player = entt::null;
	std::unique_ptr<Engine::BulletPool> enemyBulletPool;
	std::unique_ptr<Engine::BulletPool> playerBulletPool;
	Engine::Stage stage;

	bool playing = false;
	bool gameOver = false;
	int enemyDamagePerHit = 10;
	int scorePerKill = 100;

public:
	// fixed regardless of the display panel's size, so resizing docks never
	// shifts the simulation's own coordinate space - only how it's displayed
	static constexpr int kWidth = 1280;
	static constexpr int kHeight = 720;

	explicit EmbeddedGame(const std::string& fontPath);

	// saves stageDefinition to stagePath, then loads and starts simulating it -
	// so Play always previews the stage as it currently is, including unsaved edits
	void Start(const StageDefinition& stageDefinition, const std::string& stagePath);
	void Stop();
	bool IsPlaying() const { return playing; }

	// inputAllowed gates WASD/Space reads (e.g. only while the viewport panel
	// has focus) - the simulation itself keeps advancing either way
	void Update(float deltaTime, bool inputAllowed);
	void Render();

	unsigned int GetColorTextureId() const { return framebuffer.GetColorTextureId(); }
	int GetWidth() const { return framebuffer.GetWidth(); }
	int GetHeight() const { return framebuffer.GetHeight(); }
};

}
