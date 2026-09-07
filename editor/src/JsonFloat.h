#pragma once

#include <cmath>

namespace Editor {

// Rounds to 6 decimal places before storing in JSON. Without this, widening a
// float straight to nlohmann::json's double storage exposes the float's own
// rounding error on dump (e.g. 0.55f becomes "0.550000011920929")
inline double CleanFloat(float value){
	return std::round(static_cast<double>(value) * 1e6) / 1e6;
}

}
