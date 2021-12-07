cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

include_directories(/usr/local/include include)

add_executable(RayTracing main.cpp src/BVH.cpp src/Renderer.cpp src/Scene.cpp src/Vector.cpp)
