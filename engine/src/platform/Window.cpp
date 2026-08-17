#include "engine/platform/Window.h"
#include "engine/core/Log.h"

// glad must be included before glfw3.h
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Engine {

namespace {

int windowCount = 0;

void GlfwErrorCallback(int error, const char* description){
	ENGINE_CORE_ERROR("GLFW error {}: {}", error, description);
}

}

Window::Window(const WindowProps& props): width(props.width), height(props.height){
	if(windowCount == 0){
		glfwSetErrorCallback(GlfwErrorCallback);

		if(!glfwInit()){
			ENGINE_CORE_CRITICAL("Failed to initialize GLFW");
			return;
		}
	}
	windowCount++;

	// OpenGL 4.1 core is macOS's ceiling, so it's the ceiling everywhere
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	handle = glfwCreateWindow(width, height, props.title.c_str(), nullptr, nullptr);
	if(!handle){
		ENGINE_CORE_CRITICAL("Failed to create GLFW window");
		return;
	}

	glfwMakeContextCurrent(handle);
	glfwSwapInterval(1);

	if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))){
		ENGINE_CORE_CRITICAL("Failed to initialize GLAD");
		return;
	}

	ENGINE_CORE_INFO("Window created: {} ({}x{})", props.title, width, height);
}

Window::~Window(){
	if(handle){
		glfwDestroyWindow(handle);
	}

	windowCount--;
	if(windowCount == 0){
		glfwTerminate();
	}
}

void Window::OnUpdate(){
	glfwPollEvents();
	glfwSwapBuffers(handle);
}

bool Window::ShouldClose() const{
	return glfwWindowShouldClose(handle);
}

}
