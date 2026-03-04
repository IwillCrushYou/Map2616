cmake_minimum_required(VERSION 3.31.10)
project(
    main
    VERSION 1.0.0
    DESCRIPTION "microprocessor emulator"
    LANGUAGES C
)
set(CMAKE_C_STANDARD 11)
file(GLOB_RECURSE SRC_FILES src/*.c)
include_directories(include)
add_executable(${PROJECT_NAME} ${SRC_FILES})