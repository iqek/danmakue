#pragma once

#include <GLFW/glfw3.h>

namespace Engine {

enum class Key : int {
	W = GLFW_KEY_W,
	A = GLFW_KEY_A,
	S = GLFW_KEY_S,
	D = GLFW_KEY_D,
	Up = GLFW_KEY_UP,
	Down = GLFW_KEY_DOWN,
	Left = GLFW_KEY_LEFT,
	Right = GLFW_KEY_RIGHT,
	Space = GLFW_KEY_SPACE,
	Escape = GLFW_KEY_ESCAPE,
};

// Polls key state for the current window. 
// Init() must be called once after a Window exists, before any IsKeyPressed() calls.
class Input {
private:
	static GLFWwindow* windowHandle;

public:
	static void Init(GLFWwindow* handle);

	static bool IsKeyPressed(Key key);
};

}
