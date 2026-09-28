#pragma once

#include <filesystem>
#include <string>

namespace Engine {

inline std::filesystem::path PathFromUtf8(const std::string& utf8){
	std::u8string u8(reinterpret_cast<const char8_t*>(utf8.data()), utf8.size());
	return std::filesystem::path(u8);
}

// The way back, for code that keeps paths around as plain utf8 strings
inline std::string Utf8FromPath(const std::filesystem::path& path){
	std::u8string u8 = path.u8string();
	return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
}

}
