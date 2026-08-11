add_library(danmaku_engine STATIC
	src/core/Log.cpp
	src/core/Time.cpp
	src/platform/Window.cpp
	src/platform/Input.cpp
	src/render/Shader.cpp
	src/render/Texture.cpp
	src/render/TextureLibrary.cpp
	src/render/Renderer2D.cpp
	src/render/RenderSystem.cpp
	src/scene/CollisionSystem.cpp
	src/danmaku/BulletPool.cpp
	src/danmaku/RadialBurstEmitter.cpp
	src/danmaku/SpiralEmitter.cpp
	src/danmaku/AimedShotEmitter.cpp
	src/danmaku/RandomScatterEmitter.cpp
)

target_include_directories(danmaku_engine PUBLIC include)
target_compile_features(danmaku_engine PUBLIC cxx_std_20)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(danmaku_engine PUBLIC
	glfw
	glm::glm
	EnTT::EnTT
	spdlog::spdlog
	glad
	stb_image
)

if(WIN32)
	target_link_libraries(danmaku_engine PUBLIC opengl32)
elseif(APPLE)
	target_link_libraries(danmaku_engine PUBLIC "-framework OpenGL")
endif()
