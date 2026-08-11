#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace Engine {

// engine code logs through the core logger,
// sandbox/editor/game code logs through the client logger
class Log {
private:
	static std::shared_ptr<spdlog::logger> coreLogger;
	static std::shared_ptr<spdlog::logger> clientLogger;

public:
	static void Init();

	static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return coreLogger; }
	static std::shared_ptr<spdlog::logger>& GetClientLogger() { return clientLogger; }
};

}

#define ENGINE_CORE_TRACE(...)::Engine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ENGINE_CORE_INFO(...)::Engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ENGINE_CORE_WARN(...)::Engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ENGINE_CORE_ERROR(...)::Engine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ENGINE_CORE_CRITICAL(...)::Engine::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define ENGINE_TRACE(...)::Engine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ENGINE_INFO(...)::Engine::Log::GetClientLogger()->info(__VA_ARGS__)
#define ENGINE_WARN(...)::Engine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...)::Engine::Log::GetClientLogger()->error(__VA_ARGS__)
#define ENGINE_CRITICAL(...)::Engine::Log::GetClientLogger()->critical(__VA_ARGS__)
