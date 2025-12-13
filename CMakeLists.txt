cmake_minimum_required(VERSION 3.16)
project(project_bc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_PYTHON_BINDINGS "Build pybind11 Python extension" ON)

# Sources
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SOURCES
    ${SRC_DIR}/gameboard.cpp
    ${SRC_DIR}/piece.cpp
    ${SRC_DIR}/move.cpp
    ${SRC_DIR}/pgn.cpp
)

add_executable(bc_example
    ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp
    ${SOURCES}
)

add_executable(bc_test_play
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_play.cpp
    ${SOURCES}
)

add_executable(bc_test_pgn
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_pgn.cpp
    ${SOURCES}
)

target_include_directories(bc_example PRIVATE ${SRC_DIR})
target_include_directories(bc_test_play PRIVATE ${SRC_DIR})
target_include_directories(bc_test_pgn PRIVATE ${SRC_DIR})

target_compile_options(bc_example PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(bc_test_play PRIVATE -Wall -Wextra -Wpedantic)

# Python extension with pybind11 (requires pybind11 package installed)
if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 CONFIG REQUIRED)
    pybind11_add_module(chess_python
        ${CMAKE_CURRENT_SOURCE_DIR}/chess_python/chess_python.cpp
        ${SOURCES}
    )
    target_include_directories(chess_python PRIVATE ${SRC_DIR})
    target_compile_options(chess_python PRIVATE -Wall -Wextra -Wpedantic)
endif()
