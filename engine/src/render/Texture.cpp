#include "engine/render/Texture.h"
#include "engine/core/Log.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Engine {

Texture::Texture(const std::string& path){
	// No vertical flip here - our mesh UVs already assume the image's natural
	// row order (top row first), matching our y-down screen convention.
	int channels = 0;
	unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
	if(!pixels){
		ENGINE_CORE_ERROR("Failed to load texture: {}", path);
		return;
	}

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	// (nearest filtering keeps pixel-art sprites crisp instead of blurring them.)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	stbi_image_free(pixels);

	ENGINE_CORE_INFO("Loaded texture: {} ({}x{})", path, width, height);
}

Texture::~Texture(){
	glDeleteTextures(1, &textureId);
}

void Texture::Bind(unsigned int slot) const{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

}
