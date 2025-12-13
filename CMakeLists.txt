cmake_minimum_required(VERSION 3.16)
project(project_bc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Clean up GCC-style warning flags when using MSVC to avoid /Wextra errors
if(MSVC)
    foreach(flagvar CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
        foreach(bad "-Wall" "-Wextra" "-Wpedantic" "/Wall" "/Wextra" "/Wpedantic")
            string(REPLACE ${bad} "" ${flagvar} "${${flagvar}}")
        endforeach()
        # Update cache to ensure regenerated projects drop the bad flags
        set(${flagvar} "${${flagvar}}" CACHE STRING "MSVC cleaned flags" FORCE)
    endforeach()
endif()

option(BUILD_PYTHON_BINDINGS "Build pybind11 Python extension" ON)

# MSVC 전용: UTF-8 인코딩 강제, 표준 예외 모델, 경고 레벨 설정
add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<CXX_COMPILER_ID:MSVC>:/EHsc>"
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
)

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

# Compiler warnings: map GCC/Clang vs MSVC
if(MSVC)
    target_compile_options(bc_example PRIVATE /utf-8 /EHsc /W4 /permissive-)
    target_compile_options(bc_test_play PRIVATE /utf-8 /EHsc /W4 /permissive-)
    target_compile_options(bc_test_pgn PRIVATE /utf-8 /EHsc /W4 /permissive-)
else()
    target_compile_options(bc_example PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(bc_test_play PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(bc_test_pgn PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Python extension with pybind11 (requires pybind11 package installed)
if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 CONFIG REQUIRED)
    pybind11_add_module(chess_python
        ${CMAKE_CURRENT_SOURCE_DIR}/chess_python/chess_python.cpp
        ${SOURCES}
    )
    target_include_directories(chess_python PRIVATE ${SRC_DIR})
    if(MSVC)
        target_compile_options(chess_python PRIVATE /utf-8 /EHsc /W4 /permissive-)
    else()
        target_compile_options(chess_python PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
