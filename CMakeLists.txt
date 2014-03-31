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

add_library(nomain STATIC ${SOURCES})

add_executable (
    lzw
    MACOSX_BUNDLE
    ${MAIN}
)

target_link_libraries(lzw nomain)

######### Build Tests
set(TESTSRC
    test/Dictionary.check
    test/LZW.check
)


foreach(_file ${TESTSRC})
    string(REPLACE ".check" ".c" _genfile ${_file})
    string(REPLACE ".check" "" _binary ${_file})
    string(REGEX REPLACE "(^.*)/([^/]*)\$" "${CMAKE_BINARY_DIR}/\\1" _dir ${_file})
    add_custom_command(
        OUTPUT ${_dir}
        COMMAND mkdir
        ARGS -p ${_dir}
    )
    add_custom_command(
        OUTPUT ${_genfile}
        COMMAND checkmk
        ARGS ${CMAKE_SOURCE_DIR}/${_file} > ${CMAKE_BINARY_DIR}/${_genfile}
        DEPENDS ${_file} ${_dir}
    )
    add_executable(${_binary} ${_genfile} ${SOURCES})
    target_link_libraries(${_binary} ${CHECK_LIBRARIES} nomain)
    add_test(${_binary} ${_binary})
endforeach()
