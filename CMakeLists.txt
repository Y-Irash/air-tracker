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
    core/target.cpp
    core/tracker.cpp
    core/io.cpp
    core/conflict.cpp
    core/point.h
    core/target.h
    core/tracker.h
    core/io.h
    core/conflict.h
)

# include-каталог ядра «наружу»: любой, кто линкует core, получает -I.../core
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core)

# данные рядом с исполняемыми файлами, чтобы "data/sample.csv" находился при запуске из build/
function(copy_data target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/data
                $<TARGET_FILE_DIR:${target}>/data
        COMMENT "копирую data/ рядом с ${target}")
endfunction()

# --- консольная проверка ядра ---
add_executable(console
    console/main.cpp
    console/console_utf8.h
    console/point_print.cpp
    console/point_print.h
)

# чтобы из файлов консоли работал #include "core/point.h"
target_include_directories(console PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(console PRIVATE core)
copy_data(console)

# --- тесты ядра ---
add_executable(core_tests tests/test_core.cpp)
target_link_libraries(core_tests PRIVATE core)
copy_data(core_tests)

enable_testing()
add_test(NAME core_tests COMMAND core_tests WORKING_DIRECTORY $<TARGET_FILE_DIR:core_tests>)

# --- интерфейс на Qt Widgets (если Qt найден) ---
find_package(Qt5 COMPONENTS Widgets QUIET)

if (Qt5_FOUND)
    set(CMAKE_AUTOMOC ON)

    add_executable(air_tracker_gui
        gui/main.cpp
        gui/mainwindow.cpp
        gui/mainwindow.h
        gui/radarwidget.cpp
        gui/radarwidget.h
    )

    target_include_directories(air_tracker_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(air_tracker_gui PRIVATE core Qt5::Widgets)
    copy_data(air_tracker_gui)

    # утилита для скриншотов README: включается флагом, по умолчанию не собирается
    option(AIR_TRACKER_SCREENSHOT "Собрать утилиту screenshot_tool для скриншотов README" OFF)
    if (AIR_TRACKER_SCREENSHOT)
        add_executable(screenshot_tool
            tools/screenshot.cpp
            gui/mainwindow.cpp
            gui/mainwindow.h
            gui/radarwidget.cpp
            gui/radarwidget.h
        )
        target_include_directories(screenshot_tool PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/gui)
        target_link_libraries(screenshot_tool PRIVATE core Qt5::Widgets)
        copy_data(screenshot_tool)
        message(STATUS "screenshot_tool включён: запускать с QT_QPA_PLATFORM=offscreen")
    endif()
else()
    message(STATUS "Qt5 не найден: цель air_tracker_gui пропущена (ядро и консоль собираются без Qt)")
endif()
