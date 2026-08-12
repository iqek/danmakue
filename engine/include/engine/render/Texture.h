#pragma once

#include <string>

namespace Engine {

// A 2D texture loaded from an image file and uploaded to the GPU.
class Texture {
private:
	unsigned int textureId = 0;
	int width = 0;
	int height = 0;

	void Upload(const unsigned char* pixels, int channels);

public:
	explicit Texture(const std::string& path);

	// Builds a texture directly from pixel data already in memory (e.g. a
	// baked font atlas) instead of loading a file. channels selects the GL
	// format: 1 = single-channel (GL_RED), 4 = GL_RGBA.
	Texture(const unsigned char* pixels, int width, int height, int channels);

	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	void Bind(unsigned int slot = 0) const;

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
};

}
