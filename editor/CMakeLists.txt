add_executable(editor
	src/main.cpp
	src/LogPanel.cpp
	src/LogSink.cpp
	src/AssetBrowserPanel.cpp
	src/DefaultLayout.cpp
	src/EmbeddedGame.cpp
	src/EmitterListEditor.cpp
	src/HierarchyPanel.cpp
	src/InspectorPanel.cpp
	src/SceneOverlay.cpp
	src/StageDefinition.cpp
	src/StagePanel.cpp
	src/TimelinePanel.cpp
)

target_link_libraries(editor PRIVATE danmaku_engine)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(editor PRIVATE imgui::imgui)

# points the asset browser at the real, editable content directory, not a build copy
target_compile_definitions(editor PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/sandbox/assets")
