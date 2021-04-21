cmake_minimum_required(VERSION 3.0)
project(YING_YANG)

find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(glfw3 3.2 REQUIRED)
find_package(assimp REQUIRED)

if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
	message(FATAL_ERROR "You need to create a build directory and run 'cmake ..'")
endif()

set(ALL_LIBS
	${OPENGL_LIBRARY}
	${ASSIMP_LIBRARIES}
	GLEW::GLEW
	glfw
)

add_executable(yingyang
	main.cpp
	controls.cpp
	loadBmp.cpp
)
target_link_libraries(yingyang
	${ALL_LIBS}
)

