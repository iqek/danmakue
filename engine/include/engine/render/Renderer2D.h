#pragma once

#include <glm/glm.hpp>

#include "engine/render/Shader.h"
#include "engine/render/Texture.h"

namespace Engine {

// Draws 2D quads
// One draw call per quad for now 
// batching thousands of bullets in one call is a later task.
class Renderer2D {
private:
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ebo = 0;
	glm::mat4 viewProjection{ 1.0f };
	Shader colorShader;
	Shader textureShader;

public:
	Renderer2D(int viewportWidth, int viewportHeight);
	~Renderer2D();

	Renderer2D(const Renderer2D&) = delete;
	Renderer2D& operator=(const Renderer2D&) = delete;

	void Clear(const glm::vec4& color) const;
	void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) const;
	void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Texture& texture, const glm::vec4& tint = glm::vec4(1.0f)) const;
};

}
