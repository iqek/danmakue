#pragma once

#include <memory>
#include <vector>

#include "engine/danmaku/emitters/BulletEmitter.h"
#include "engine/danmaku/movement/MovementPattern.h"

namespace Engine {

// A stage enemy. HP, the patterns it's firing, and how it moves.
struct Enemy {
	int health = 1;
	int maxHealth = 1;
	std::vector<std::unique_ptr<BulletEmitter>> emitters;
	std::unique_ptr<MovementPattern> movement;

	Enemy() = default;
	Enemy(const Enemy&) = delete;
	Enemy& operator=(const Enemy&) = delete;
	Enemy(Enemy&&) = default;
	Enemy& operator=(Enemy&&) = default;
};

}
