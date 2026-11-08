cmake_minimum_required(VERSION 3.20)

project(kibnes LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)

add_subdirectory(vendor/SDL)

add_library(imgui STATIC
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/backends/imgui_impl_sdl3.cpp
    vendor/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui PUBLIC vendor/imgui vendor/imgui/backends)

target_link_libraries(imgui PUBLIC SDL3::SDL3)


file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS src/core/*.cpp)

add_library(nescore STATIC ${CORE_SOURCES})

target_include_directories(nescore PUBLIC src)


file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS src/frontend/*.cpp)

add_executable(kibnes ${APP_SOURCES})

target_link_libraries(kibnes PRIVATE nescore imgui SDL3::SDL3)


if(MSVC)
    target_compile_options(nescore PRIVATE /W4)
    target_compile_options(kibnes  PRIVATE /W4)
else()
    target_compile_options(nescore PRIVATE -Wall -Wextra)
    target_compile_options(kibnes  PRIVATE -Wall -Wextra)

endif()