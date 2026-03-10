cmake_minimum_required(VERSION 3.16)
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

if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/frontend/client/src/wasm"
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sENVIRONMENT=web,worker"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sSTACK_SIZE=1048576"
        "-sINVOKE_RUN=0"
        "-sEXPORTED_RUNTIME_METHODS=['FS','callMain']"
    )
endif()
