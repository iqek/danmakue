#pragma once

#include <filesystem>
#include <string>

namespace Engine {

inline std::filesystem::path PathFromUtf8(const std::string& utf8){
	std::u8string u8(reinterpret_cast<const char8_t*>(utf8.data()), utf8.size());
	return std::filesystem::path(u8);
}

}
