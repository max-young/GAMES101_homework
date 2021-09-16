# CMAKE最下版本要求
cmake_minimum_required(VERSION 3.10)
# 工程名称
project(Rasterizer)

# 引入非官方包OpenCV
find_package(OpenCV REQUIRED)

# 定义变量CMAKE_CXX_STANDARD的值为17
set(CMAKE_CXX_STANDARD 17)

# 自定义编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# 编译可进行调试的文件
set(CMAKE_BUILD_TYPE Debug)

# 头文件搜索路径
include_directories(/usr/local/include include)

# 生成可执行文件, 将5个cpp文件生成一个名叫Rasterizer的可执行文件
add_executable(Rasterizer main.cpp src/rasterizer.cpp src/Triangle.cpp)
# 添加需要链接的共享库, 将opencv的库链接到可执行文件Rasterizer
target_link_libraries(Rasterizer ${OpenCV_LIBRARIES})
