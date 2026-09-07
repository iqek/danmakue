add_executable(sandbox
	src/main.cpp
)

target_link_libraries(sandbox PRIVATE danmaku_engine)

# stage1.json is loaded straight from source so Play always reflects the
# editor's last save, not a stale build-time copy - other assets still copy below
target_compile_definitions(sandbox PRIVATE ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

add_custom_command(TARGET sandbox POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/assets
		$<TARGET_FILE_DIR:sandbox>/assets
)
