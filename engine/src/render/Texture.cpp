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

	Upload(pixels, 4);
	stbi_image_free(pixels);

	ENGINE_CORE_INFO("Loaded texture: {} ({}x{})", path, width, height);
}

Texture::Texture(const unsigned char* pixels, int width, int height, int channels):
	width(width), height(height)
{
	Upload(pixels, channels);
}

void Texture::Upload(const unsigned char* pixels, int channels){
	unsigned int format = channels == 1 ? GL_RED : GL_RGBA;
	unsigned int internalFormat = channels == 1 ? GL_R8 : GL_RGBA8;

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	// Nearest filtering keeps pixel-art sprites (and font glyphs) crisp
	// instead of blurring them.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(channels == 1){
		// A single-channel texture (e.g. a font atlas) only has red populated.
		// Swizzling all four channels to read from red means the existing
		// tint-multiply shader "just works": color = tint, alpha = coverage.
		int swizzle[] = { GL_RED, GL_RED, GL_RED, GL_RED };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(internalFormat), width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
}

Texture::~Texture(){
	glDeleteTextures(1, &textureId);
}

void Texture::Bind(unsigned int slot) const{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

}
