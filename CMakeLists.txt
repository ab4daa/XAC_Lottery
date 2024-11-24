cmake_minimum_required(VERSION 3.16)
project(XAC_Lottery)

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!!!!!                                                                            !!!!!!
# !!!!!!     This CMake script is not using "CMake best practices".                 !!!!!!
# !!!!!!                 Don't use it in your project.                              !!!!!!
# !!!!!!                                                                            !!!!!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


# 3. Download SDL, and use that.
include(FetchContent)
set(SDL_SHARED TRUE CACHE BOOL "Build a SDL shared library (if available)")
set(SDL_STATIC TRUE CACHE BOOL "Build a SDL static library (if available)")
FetchContent_Declare(
	SDL
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG preview-3.1.6  # Replace this with a particular git tag or git hash
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)
message(STATUS "Using SDL3 via FetchContent")
#FetchContent_MakeAvailable(SDL)
#set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-src" PROPERTY EXCLUDE_FROM_ALL TRUE)

set(BUILD_SHARED_LIBS FALSE)
FetchContent_Declare(
	SDL_image
	GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
	GIT_TAG main  # Replace this with a particular git tag or git hash
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)
message(STATUS "Using SDL_image via FetchContent")
FetchContent_MakeAvailable(SDL SDL_image)
#set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-src" PROPERTY EXCLUDE_FROM_ALL TRUE)
#set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl_image-src" PROPERTY EXCLUDE_FROM_ALL TRUE)


add_executable(XAC_Lottery WIN32 main.cpp Candidate.cpp Candidate.h Logging.cpp Logging.h)

target_link_libraries(XAC_Lottery PRIVATE SDL3::SDL3-static SDL3_image-static)