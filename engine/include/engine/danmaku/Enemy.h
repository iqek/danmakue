#pragma once

#include <memory>
#include <vector>

#include "engine/danmaku/BulletEmitter.h"
#include "engine/danmaku/MovementPattern.h"

namespace Engine {

// A stage enemy: HP, the patterns it's currently firing, and how it moves.
// Deliberately has no concept of "boss" or "spell card" - those are content
// built on top later (bigger health, a health bar, phase-swapping the
// emitters/movement below), not separate engine types.
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
