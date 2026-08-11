#include "engine/core/Time.h"

#include <GLFW/glfw3.h>

namespace Engine {

double Time::lastFrameTime = 0.0;
float Time::deltaTime = 0.0f;
float Time::elapsedTime = 0.0f;

void Time::Update(){
	double currentTime = glfwGetTime();
	deltaTime = static_cast<float>(currentTime - lastFrameTime);
	lastFrameTime = currentTime;
	elapsedTime += deltaTime;
}

}
