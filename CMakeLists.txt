cmake_minimum_required(VERSION 3.10)
project(warp)

# 使用 C++17 標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


#include main,side
# 指定把 src/main.cpp 編譯成名為 warp 的執行檔
add_executable(warp src/main.cpp src/side.cpp)
