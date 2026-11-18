cmake_minimum_required(VERSION 3.16)

project(air_tracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# предупреждения, как договорились в правилах проекта
add_compile_options(-Wall -Wextra)

# --- ядро: чистый C++, без Qt ---
add_library(core STATIC
    core/point.cpp
    core/point.h
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core)

# --- консольная проверка ядра ---
add_executable(console console/main.cpp
    console/console_utf8.h)
target_link_libraries(console PRIVATE core)
