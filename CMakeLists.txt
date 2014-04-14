cmake_minimum_required(VERSION 2.8)
Project(life-gif)

option(SHARED_LIB "Build as a shared library" ON)

set(MAIN
    src/main.c
)

set(SOURCES
    src/LZW.c
    src/Dictionary.c
    src/Gif.c
)

set(TESTSRC
    test/Dictionary.check
    test/LZW.check
    test/Gif.check
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

#math library, for log
if(UNIX)
    target_link_libraries(tinygif m)
endif(UNIX)

add_executable(
    tinygif-example
    MACOSX_BUNDLE
    ${MAIN}
)

target_link_libraries(tinygif-example tinygif)

install(TARGETS tinygif LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/Gif.h DESTINATION include)

######### Build Tests
enable_testing()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(Check REQUIRED)

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
    target_link_libraries(${_binary} ${CHECK_LIBRARIES} tinygif)
    add_test(${_binary} ${_binary})
endforeach()
