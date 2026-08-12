add_library(stb_truetype STATIC
	src/stb_truetype.cpp
)

target_include_directories(stb_truetype PUBLIC include)
