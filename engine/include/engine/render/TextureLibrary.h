#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "engine/render/Texture.h"

namespace Engine {

// Loads each texture file once and hands out shared ownership of it
// so the same image isn't loaded to the GPU twice
class TextureLibrary {
private:
	static std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

public:
	static std::shared_ptr<Texture> Get(const std::string& path);
};

}
