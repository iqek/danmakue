#pragma once

#include <nlohmann/json.hpp>

namespace Editor {

// A list of emitter definitions with add/remove and per-type field editing.
// Shared between enemy emitters and player weapons - same JSON shape for both
void DrawEmitterList(nlohmann::json& emitters);

}
