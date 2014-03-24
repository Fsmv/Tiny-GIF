cmake_minimum_required(VERSION 2.8)
Project(life-gif)

set(SOURCES
    src/main.c
    src/LZW.c
    src/Dictionary.c
    src/Gif.c
)

include_directories (
    ${CMAKE_SOURCE_DIR}/include
)

add_executable (
    life-gif
    WIN32
    MACOSX_BUNDLE
    ${SOURCES}
)

#math library, for log
if(UNIX)
    target_link_libraries(life-gif m)
endif(UNIX)
