#pragma once

#include <filesystem>
#include <string>

namespace Editor {

// Launches the game as a separate process, and can stop it again
class PlayController {
private:
	std::filesystem::path executablePath;

#ifdef _WIN32
	void* processHandle = nullptr;
#else
	int processId = -1;
#endif

public:
	explicit PlayController(const std::string& utf8ExecutablePath);
	~PlayController();

	PlayController(const PlayController&) = delete;
	PlayController& operator=(const PlayController&) = delete;

	// stageFileName is passed to the game as a bare filename, not a full path -
	// a full path would carry non-ASCII characters (this project's username)
	// through argv, which Windows' narrow-argv conversion mangles
	void Launch(const std::string& stageFileName);
	void Stop();
	bool IsRunning();

	void Draw(const char* title, const std::string& stageFileName);
};

}
