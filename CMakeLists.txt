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

# include-каталог ядра «наружу»: любой, кто линкует core, получает -I.../core
# и может писать #include "point.h"
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core)

# --- консольная проверка ядра ---
add_executable(console
    console/main.cpp
    console/console_utf8.h
    console/point_print.cpp
    console/point_print.h
)

# чтобы из файлов консоли работал #include "core/point.h"
# (корень проекта в списке include-каталогов: путь выглядит как от корня)
target_include_directories(console PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(console PRIVATE core)
