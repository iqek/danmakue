#pragma once

#include <memory>
#include <vector>

#include "engine/danmaku/emitters/BulletEmitter.h"

namespace Engine {

// weapons is a list, not a fixed emitter
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
