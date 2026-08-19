cmake_minimum_required(VERSION 3.22)
project(DeadlockModManager VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find System OpenGL
find_package(OpenGL REQUIRED)

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# SQLite3
FetchContent_Declare(
    sqlite3
    URL https://sqlite.org/2026/sqlite-amalgamation-3530400.zip
)
FetchContent_MakeAvailable(sqlite3)

add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.4
)
FetchContent_MakeAvailable(imgui)

# Explicitly list core imgui sources along with the backends
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Project targets
file(GLOB_RECURSE STORAGE_SOURCES src/storage/*.cpp)
add_library(app_storage ${STORAGE_SOURCES})
target_include_directories(app_storage PUBLIC include)
target_link_libraries(app_storage PUBLIC sqlite3)

file(GLOB_RECURSE MANAGER_SOURCES src/manager/*.cpp)
add_library(app_manager ${MANAGER_SOURCES})
target_include_directories(app_manager PUBLIC include)
target_link_libraries(app_manager PUBLIC app_storage)

file(GLOB_RECURSE UI_SOURCES src/ui/*.cpp)
add_library(app_ui ${UI_SOURCES})
target_include_directories(app_ui PUBLIC include src/ui)
target_link_libraries(app_ui PUBLIC app_manager imgui)

add_executable(DeadlockModManager src/main.cpp)
target_link_libraries(DeadlockModManager PRIVATE app_ui app_manager app_storage imgui OpenGL::GL)