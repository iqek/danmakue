#pragma once

namespace Engine {

// An offscreen render target with a single color attachment, e.g. for
// rendering a game view into an ImGui panel instead of the main window
class Framebuffer {
private:
	unsigned int framebufferId = 0;
	unsigned int colorTextureId = 0;
	int width = 0;
	int height = 0;

	void CreateColorTexture();

public:
	Framebuffer(int width, int height);
	~Framebuffer();

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;

	void Bind() const;
	void Unbind() const;

	// no-op if the size hasn't actually changed
	void Resize(int newWidth, int newHeight);

	unsigned int GetColorTextureId() const { return colorTextureId; }
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
};

}
