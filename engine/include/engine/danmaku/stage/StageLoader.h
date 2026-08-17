#pragma once

#include <string>

#include "engine/danmaku/stage/Stage.h"

namespace Engine {

// Loads a Stage from a JSON file: enemy spawn triggers, movement, emitters
Stage LoadStage(const std::string& path);

}
