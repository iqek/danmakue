#include "engine/render/Renderer2D.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

namespace {

const char* colorVertexSource = R"(
#version 410 core
layout(location = 0) in vec2 aPosition;

uniform mat4 uViewProjection;
uniform mat4 uModel;

void main(){
	gl_Position = uViewProjection * uModel * vec4(aPosition, 0.0, 1.0);
}
)";

const char* colorFragmentSource = R"(
#version 410 core
out vec4 FragColor;

uniform vec4 uColor;

void main(){
	FragColor = uColor;
}
)";

const char* textureVertexSource = R"(
#version 410 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;

uniform mat4 uViewProjection;
uniform mat4 uModel;

out vec2 vUV;

void main(){
	vUV = aUV;
	gl_Position = uViewProjection * uModel * vec4(aPosition, 0.0, 1.0);
}
)";

const char* textureFragmentSource = R"(
#version 410 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uTint;

void main(){
	FragColor = texture(uTexture, vUV) * uTint;
}
)";

}

Renderer2D::Renderer2D(int viewportWidth, int viewportHeight):
	colorShader(colorVertexSource, colorFragmentSource),
	textureShader(textureVertexSource, textureFragmentSource)
{
	// each vertex is position.xy + uv.xy 
	// the color shader only reads location 0, the texture shader reads both
	float vertices[] = {
		-0.5f, -0.5f,   0.0f, 0.0f,
		 0.5f, -0.5f,   1.0f, 0.0f,
		 0.5f,  0.5f,   1.0f, 1.0f,
		-0.5f,  0.5f,   0.0f, 1.0f,
	};
	unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	viewProjection = glm::ortho(0.0f, static_cast<float>(viewportWidth), static_cast<float>(viewportHeight), 0.0f, -1.0f, 1.0f);
}

Renderer2D::~Renderer2D(){
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void Renderer2D::Clear(const glm::vec4& color) const{
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) const{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));

	colorShader.Bind();
	colorShader.SetMat4("uViewProjection", viewProjection);
	colorShader.SetMat4("uModel", model);
	colorShader.SetVec4("uColor", color);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Texture& texture, const glm::vec4& tint) const{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));

	textureShader.Bind();
	textureShader.SetMat4("uViewProjection", viewProjection);
	textureShader.SetMat4("uModel", model);
	textureShader.SetVec4("uTint", tint);
	textureShader.SetInt("uTexture", 0);

	texture.Bind(0);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

}
