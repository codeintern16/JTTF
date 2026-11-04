cmake_minimum_required(VERSION 3.10)
project(JTTF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 1. 建立 Cargo 編譯指令
add_custom_target(cargo_build ALL
    COMMAND cargo build --release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/jttf-matcher
)

add_executable(warp src/main.cpp src/side.cpp)

# 2. 確保在編譯 C++ 前，先編譯好 Rust
add_dependencies(warp cargo_build)

# 3. 連結 Rust 靜態庫與系統依賴
target_link_libraries(warp PRIVATE
    ${CMAKE_SOURCE_DIR}/jttf-matcher/target/release/libjttf_matcher.a
    dl pthread m
)

install(TARGETS warp DESTINATION bin)
install(FILES scripts/warp.sh DESTINATION share/warp)
