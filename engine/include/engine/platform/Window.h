#pragma once

#include <string>

struct GLFWwindow;

namespace Engine {

struct WindowProps {
	std::string title = "Danmakue";
	int width = 1280;
	int height = 720;
};

// GLFW is initialized on the first Window, shut down on the last
class Window {
private:
	GLFWwindow* handle = nullptr;
	int width = 0;
	int height = 0;

public:
	explicit Window(const WindowProps& props = WindowProps());
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	// Polls input and presents the frame. call once per loop iteration
	void OnUpdate();
	bool ShouldClose() const;

	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
	GLFWwindow* GetHandle() const { return handle; }
};

}
