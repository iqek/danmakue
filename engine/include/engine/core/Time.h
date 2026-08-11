#pragma once

namespace Engine {

// Tracks per-frame delta time and total elapsed time. 
// call Update() once per frame (before anything that reads GetDeltaTime()/GetElapsedTime() that frame)
class Time {
private:
	static double lastFrameTime;
	static float deltaTime;
	static float elapsedTime;

public:
	static void Update();

	static float GetDeltaTime() { return deltaTime; }
	static float GetElapsedTime() { return elapsedTime; }
};

}
