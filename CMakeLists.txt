cmake_minimum_required(VERSION 2.8)

project(radar-strike)

option(BUILD_TESTS "Also build tests" OFF)

include(win-cpp-deps.cmake/win-cpp-deps.cmake)

install_dep("https://bitbucket.org/wincppdeps/sdl2.git")
install_dep("https://bitbucket.org/wincppdeps/glm.git")
install_dep("https://bitbucket.org/wincppdeps/glextloader.git")

if(WIN32)
	find_package(OpenGL REQUIRED)
endif()

find_package(GLM REQUIRED)
find_package(SDL2 REQUIRED)

set(SRC_ASTAR
	src/astar.cpp
	)

set(SRC_APP
	src/impl-headers.cpp
	src/program.cpp
	src/sdl2-setup.cpp
	src/input.cpp
	src/log.cpp
	src/players.cpp
	src/ui/ui.cpp
    )

set(HDR_APP
	src/sdl2-setup.h
	src/input.h
	src/iinput.h
	src/font-icons.h
	src/log.h
	src/players.h
	src/platform.h
	src/ui/ui.h
	src/ui/gamemodes.h
	libs/gl.utilities/gl.utilities.shaders.h
	libs/gl.utilities/gl.utilities.textures.h
	libs/gl.utilities/gl.utilities.vertexbuffers.h
    )

set(NANOVG
	libs/nanovg/src/nanovg.c
	libs/nanovg/src/nanovg.h
	libs/nanovg/src/nanovg_gl.h
	)

#set(SHADERS
#	data/shaders/gl3/geometry.vert
#	data/shaders/gl3/geometry.frag
#	data/shaders/gles3/geometry.vert
#	data/shaders/gles3/geometry.frag
#	)

add_executable(radar-strike
	${SRC_APP}
	${SRC_ASTAR}
	${HDR_APP}
	${NANOVG}
#	${SHADERS}
	)

target_include_directories(radar-strike
	PRIVATE ${GLM_INCLUDE_DIR}
	PRIVATE ${SDL2_INCLUDE_DIR}
	PRIVATE "${CMAKE_SOURCE_DIR}/libs/gl.utilities"
	PRIVATE "${CMAKE_SOURCE_DIR}/libs/nanovg/src"
	)

target_compile_features(radar-strike
	PRIVATE cxx_auto_type
	PRIVATE cxx_nullptr
	PRIVATE cxx_range_for
	)

target_link_libraries(radar-strike
	${SDL2_LIBRARY}
	${OPENGL_LIBRARIES}
)

if(BUILD_TESTS)

	enable_testing()

	add_executable(all-tests
		tests/catch.hpp
		tests/test-astar.cpp
		tests/test-players.cpp
		tests/test-base.cpp
		${SRC_ASTAR}
		src/players.cpp
		)

	target_compile_features(all-tests
		PRIVATE cxx_auto_type
		PRIVATE cxx_nullptr
		PRIVATE cxx_range_for
		)

	target_include_directories(all-tests
		PRIVATE src
		PRIVATE ${GLM_INCLUDE_DIR}
		PRIVATE "${CMAKE_SOURCE_DIR}/libs/gl.utilities"
		PRIVATE "${CMAKE_SOURCE_DIR}/libs/nanovg/src"
		)

	target_link_libraries(all-tests
		${SDL2_LIBRARY}
		${OPENGL_LIBRARIES}
		)

	add_test(
		NAME all_tests
		COMMAND all-tests
		)

endif(BUILD_TESTS)
