#include "engine/platform/Input.h"

namespace Engine {

GLFWwindow* Input::windowHandle = nullptr;

void Input::Init(GLFWwindow* handle){
	windowHandle = handle;
}

bool Input::IsKeyPressed(Key key){
	int state = glfwGetKey(windowHandle, static_cast<int>(key));
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

}
