cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# 编译可进行调试的文件
set(CMAKE_BUILD_TYPE Debug)

include_directories(/usr/local/include include)

add_executable(Rasterizer main.cpp src/rasterizer.cpp src/Triangle.cpp src/Texture.cpp)
target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})
#target_compile_options(Rasterizer PUBLIC -Wall -Wextra -pedantic)