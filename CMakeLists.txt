cmake_minimum_required(VERSION 3.20.5)

project(te)
        
enable_language(C CXX ASM)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_package(raylib REQUIRED)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0
)
FetchContent_MakeAvailable(yaml-cpp)


add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} raylib yaml-cpp::yaml-cpp)

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE
  -Wold-style-cast
  -Wall
  -Wpedantic
  -Wextra
  -Werror
  -Wshadow
  $<$<CONFIG:Debug>:-Og>
  -Wcast-align
  -Wunused
  -Woverloaded-virtual
  -Wsign-conversion
  -Wno-narrowing
  -Wnull-dereference
  -Wdouble-promotion
  -Wformat=2
  -Wmisleading-indentation
  -Wduplicated-cond
  -Wduplicated-branches
  -Wlogical-op
  -Wuseless-cast
)
