#include "engine/render/TextureLibrary.h"

namespace Engine {

std::unordered_map<std::string, std::shared_ptr<Texture>> TextureLibrary::textures;

std::shared_ptr<Texture> TextureLibrary::Get(const std::string& path){
	auto it = textures.find(path);
	if(it != textures.end()){
		return it->second;
	}

	auto texture = std::make_shared<Texture>(path);
	textures[path] = texture;
	return texture;
}

}
