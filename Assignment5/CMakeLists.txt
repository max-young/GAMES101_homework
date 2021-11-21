cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

# 编译可进行调试的文件
set(CMAKE_BUILD_TYPE Debug)

include_directories(/usr/local/include include)

add_executable(RayTracing main.cpp src/Scene.cpp src/Renderer.cpp)
target_compile_options(RayTracing PUBLIC -Wall -Wextra -pedantic -Wshadow -Wreturn-type -fsanitize=undefined)
target_compile_features(RayTracing PUBLIC cxx_std_17)
target_link_libraries(RayTracing PUBLIC -fsanitize=undefined)
