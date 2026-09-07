#pragma once

#include <string>

#include "engine/danmaku/stage/Stage.h"

namespace Engine {

// Loads a Stage from a JSON file: enemy spawn triggers, movement, emitters
Stage LoadStage(const std::string& path);

// Reads the same file's "player" key and spawns the player entity from it.
// Falls back to sensible defaults if "player" (or any of its fields) is missing
entt::entity SpawnPlayer(entt::registry& registry, const std::string& path);

}
