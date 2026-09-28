#include "engine/render/Texture.h"
#include "engine/core/Log.h"
#include "engine/core/Utf8Path.h"

#include <cstddef>
#include <fstream>
#include <glad/glad.h>
#include <stb_image.h>
#include <vector>

namespace Engine {

Texture::Texture(const std::string& path){
	// the bytes are read here rather than by stb: a narrow path would be decoded
	// with the ANSI codepage and mangle any non-ASCII folder name on Windows
	std::vector<unsigned char> bytes;
	std::ifstream file(PathFromUtf8(path), std::ios::binary | std::ios::ate);
	if(file){
		bytes.resize(static_cast<std::size_t>(file.tellg()));
		file.seekg(0);
		file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
	}

	// No vertical flip: our mesh UVs already assume top row first
	int channels = 0;
	unsigned char* pixels = bytes.empty() ? nullptr : stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &width, &height, &channels, 4);
	if(!pixels){
		ENGINE_CORE_ERROR("Failed to load texture: {}", path);
		// magenta, so a missing texture is obvious instead of drawing nothing at all
		static const unsigned char missing[4] = { 255, 0, 255, 255 };
		width = 1;
		height = 1;
		Upload(missing, 4);
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

	// Nearest filtering keeps pixel art and glyphs crisp, not blurry
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(channels == 1){
		// Swizzle red into all channels so the tint-multiply shader still works
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
