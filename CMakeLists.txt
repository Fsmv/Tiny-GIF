cmake_minimum_required(VERSION 2.8)
Project(life-gif)

include(ExternalProject)
ExternalProject_Add(tinygif
        GIT_REPOSITORY https://github.com/Fsmv/Tiny-GIF.git
        INSTALL_DIR ${PROJECT_BINARY_DIR}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

if(UNIX)
    set(PREFIX "lib")
    set(SUFFIX ".so")
elseif(APPLE)
    set(PREFIX "lib")
    set(SUFFIX ".so")
else()
    set(PREFIX "")
    set(SUFFIX ".dll")
endif()

set(TINYGIF_LIBS ${PROJECT_BINARY_DIR}/lib/${PREFIX}tinygif${SUFFIX})

set(SOURCES
    src/main.c
)

include_directories (
    ${CMAKE_SOURCE_DIR}/include
)

add_executable (
    life-gif
    MACOSX_BUNDLE
    ${SOURCES}
)

add_dependencies(life-gif tinygif)

target_link_libraries(life-gif ${TINYGIF_LIBS})

#math library, for log
if(UNIX)
    target_link_libraries(life-gif m)
endif(UNIX)
