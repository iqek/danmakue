add_library(danmaku_engine STATIC
	src/core/Log.cpp
	src/core/Time.cpp
	src/platform/Window.cpp
	src/platform/Input.cpp
	src/render/Shader.cpp
	src/render/Texture.cpp
	src/render/TextureLibrary.cpp
	src/render/Font.cpp
	src/render/Renderer2D.cpp
	src/render/RenderSystem.cpp
	src/scene/CollisionSystem.cpp
	src/ui/Hud.cpp
	src/danmaku/BulletPool.cpp
	src/danmaku/EnemySystem.cpp
	src/danmaku/PlayerSystem.cpp
	src/danmaku/emitters/RadialBurstEmitter.cpp
	src/danmaku/emitters/SpiralEmitter.cpp
	src/danmaku/emitters/AimedShotEmitter.cpp
	src/danmaku/emitters/RandomScatterEmitter.cpp
	src/danmaku/emitters/StraightShotEmitter.cpp
	src/danmaku/movement/LinearMovement.cpp
	src/danmaku/movement/WaypointMovement.cpp
	src/danmaku/stage/Stage.cpp
	src/danmaku/stage/StageLoader.cpp
)

target_include_directories(danmaku_engine PUBLIC include)
target_compile_features(danmaku_engine PUBLIC cxx_std_20)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(danmaku_engine PUBLIC
	glfw
	glm::glm
	EnTT::EnTT
	spdlog::spdlog
	glad
	stb_image
	stb_truetype
	nlohmann_json::nlohmann_json
)

if(WIN32)
	target_link_libraries(danmaku_engine PUBLIC opengl32)
elseif(APPLE)
	target_link_libraries(danmaku_engine PUBLIC "-framework OpenGL")
endif()
