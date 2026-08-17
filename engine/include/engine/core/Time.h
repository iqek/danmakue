#pragma once

namespace Engine {

// Call Update() once per frame, before reading GetDeltaTime()/GetElapsedTime()
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
