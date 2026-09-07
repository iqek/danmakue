#include "PlayController.h"

#include "engine/core/Log.h"
#include "engine/core/Utf8Path.h"

#include <imgui.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace Editor {

PlayController::PlayController(const std::string& utf8ExecutablePath): executablePath(Engine::PathFromUtf8(utf8ExecutablePath)){}

PlayController::~PlayController(){
	Stop();
}

bool PlayController::IsRunning(){
#ifdef _WIN32
	if(!processHandle){
		return false;
	}

	DWORD exitCode = 0;
	if(GetExitCodeProcess(static_cast<HANDLE>(processHandle), &exitCode) && exitCode == STILL_ACTIVE){
		return true;
	}

	CloseHandle(static_cast<HANDLE>(processHandle));
	processHandle = nullptr;
	return false;
#else
	if(processId <= 0){
		return false;
	}

	int status = 0;
	if(waitpid(processId, &status, WNOHANG) == 0){
		return true;
	}

	processId = -1;
	return false;
#endif
}

void PlayController::Launch(const std::string& stageFileName){
	if(IsRunning()){
		return;
	}

	if(!std::filesystem::exists(executablePath)){
		ENGINE_CORE_ERROR("Cannot launch, executable not found: {}", executablePath.string());
		return;
	}

#ifdef _WIN32
	STARTUPINFOW startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInfo{};

	std::wstring stageArg(stageFileName.begin(), stageFileName.end());
	std::wstring commandLine = L"\"" + executablePath.wstring() + L"\" \"" + stageArg + L"\"";
	std::wstring workingDirectory = executablePath.parent_path().wstring();

	bool created = CreateProcessW(executablePath.c_str(), commandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, workingDirectory.c_str(), &startupInfo, &processInfo);
	if(!created){
		ENGINE_CORE_ERROR("Failed to launch {}", executablePath.string());
		return;
	}

	CloseHandle(processInfo.hThread);
	processHandle = processInfo.hProcess;
#else
	pid_t pid = fork();
	if(pid == 0){
		chdir(executablePath.parent_path().c_str());
		execl(executablePath.c_str(), executablePath.c_str(), stageFileName.c_str(), nullptr);
		_exit(1);
	}
	else if(pid > 0){
		processId = pid;
	}
	else{
		ENGINE_CORE_ERROR("Failed to fork for {}", executablePath.string());
	}
#endif
}

void PlayController::Stop(){
#ifdef _WIN32
	if(processHandle){
		TerminateProcess(static_cast<HANDLE>(processHandle), 0);
		CloseHandle(static_cast<HANDLE>(processHandle));
		processHandle = nullptr;
	}
#else
	if(processId > 0){
		kill(processId, SIGTERM);
		int status = 0;
		waitpid(processId, &status, 0);
		processId = -1;
	}
#endif
}

void PlayController::Draw(const char* title, const std::string& stageFileName){
	ImGui::Begin(title);

	if(IsRunning()){
		if(ImGui::Button("Stop")){
			Stop();
		}
		ImGui::SameLine();
		ImGui::TextDisabled("Running...");
	}
	else{
		if(ImGui::Button("Play")){
			Launch(stageFileName);
		}
	}

	ImGui::End();
}

}
