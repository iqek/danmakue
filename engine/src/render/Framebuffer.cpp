#include "engine/render/Framebuffer.h"
#include "engine/core/Log.h"

#include <glad/glad.h>

namespace Engine {

Framebuffer::Framebuffer(int width, int height): width(width), height(height){
	glGenFramebuffers(1, &framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

	CreateColorTexture();

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		ENGINE_CORE_ERROR("Framebuffer incomplete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer(){
	glDeleteTextures(1, &colorTextureId);
	glDeleteFramebuffers(1, &framebufferId);
}

void Framebuffer::CreateColorTexture(){
	glGenTextures(1, &colorTextureId);
	glBindTexture(GL_TEXTURE_2D, colorTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextureId, 0);
}

void Framebuffer::Bind() const{
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
	glViewport(0, 0, width, height);
}

void Framebuffer::Unbind() const{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(int newWidth, int newHeight){
	if(newWidth == width && newHeight == height){
		return;
	}
	if(newWidth <= 0 || newHeight <= 0){
		return;
	}

	width = newWidth;
	height = newHeight;

	glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
	glDeleteTextures(1, &colorTextureId);
	CreateColorTexture();

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		ENGINE_CORE_ERROR("Framebuffer incomplete after resize");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}
