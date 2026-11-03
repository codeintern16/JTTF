cmake_minimum_required(VERSION 3.10)
project(JTTF)

# 使用 C++17 標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


#include main,side
# 指定把 src/main.cpp 編譯成名為 warp 的執行檔
add_executable(warp src/main.cpp src/side.cpp)



# 1. 把編譯好的二進位檔安裝到 /usr/local/bin (系統指令區)
install(TARGETS warp DESTINATION bin)

# 2. 把 Bash 腳本安裝到 /usr/local/share/warp (系統共享資源區)
install(FILES scripts/warp.sh DESTINATION share/warp)
