cmake_minimum_required(VERSION 2.8)
Project(lzw)

enable_testing()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(Check REQUIRED)

set(MAIN
    src/main.c
)

set(SOURCES
    src/LZW.c
    src/Dictionary.c
)

include_directories (
    ${CMAKE_SOURCE_DIR}/include
)

add_executable (
    lzw
    MACOSX_BUNDLE
    ${MAIN} ${SOURCES}
)

######### Build Tests
set(TESTSRC
    test/Dictionary.check
)

set(GENSRC)
foreach(_file ${TESTSRC})
    string(REPLACE ".check" ".c" genfile ${_file})
    string(REGEX REPLACE "(^.*)/([^/]*)\$" "${CMAKE_BINARY_DIR}/\\1" FOLDER ${_file})
    add_custom_command(
        OUTPUT ${FOLDER}
        COMMAND mkdir
        ARGS -p ${FOLDER}
    )
    add_custom_command(
        OUTPUT ${genfile}
        COMMAND checkmk
        ARGS ${CMAKE_SOURCE_DIR}/${_file} > ${CMAKE_BINARY_DIR}/${genfile}
        DEPENDS ${_file} ${FOLDER}
    )
    list(APPEND GENSRC ${genfile})
endforeach()

add_executable(runtests ${GENSRC} ${SOURCES})
target_link_libraries(runtests ${CHECK_LIBRARIES})

add_test(tests runtests)
