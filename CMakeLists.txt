cmake_minimum_required(VERSION 2.8)
Project(life-gif)

option(SHARED_LIB "Build as a shared library" OFF)

set(SOURCES
    src/LZW.c
    src/Dictionary.c
    src/Gif.c
)

include_directories (
    ${CMAKE_SOURCE_DIR}/include
)

if(SHARED_LIB)
    set(LIB_TYPE SHARED)
else(SHARED_LIB)
    set(LIB_TYPE STATIC)
endif(SHARED_LIB)

add_library (
    tinygif
    ${LIB_TYPE}
    ${SOURCES}
)

add_executable(
    tinygif-example
    WIN32
    MACOSX_BUNDLE
    src/main.c
)

target_link_libraries(tinygif-example tinygif)

#math library, for log
if(UNIX)
    target_link_libraries(tinygif m)
endif(UNIX)
