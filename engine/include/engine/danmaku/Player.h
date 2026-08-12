#pragma once

#include <memory>
#include <vector>

#include "engine/danmaku/BulletEmitter.h"

namespace Engine {

// weapons is deliberately a list, not one fixed emitter - some games give
// the player several simultaneous shot types, and some player characters
// (or entire games) might have no attached weapon at all.
struct Player {
	int lives = 3;
	int score = 0;
	float invincibleTimer = 0.0f;
	std::vector<std::unique_ptr<BulletEmitter>> weapons;

	Player() = default;
	Player(const Player&) = delete;
	Player& operator=(const Player&) = delete;
	Player(Player&&) = default;
	Player& operator=(Player&&) = default;
};

}
