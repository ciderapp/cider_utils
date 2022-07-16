include(FetchContent)
include(ExternalProject)

# Include zlib-ng
set(ZLIB_ENABLE_TESTS OFF)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng
    GIT_TAG 2.0.6
)

FetchContent_MakeAvailable(zlib)

# Stolen from https://github.com/schneefux/node-taglib3
# FetchContent was being a bitch
ExternalProject_Add(
    taglib
    PREFIX "${CMAKE_BINARY_DIR}/taglib"
    GIT_REPOSITORY https://github.com/taglib/taglib
    GIT_TAG v1.12
    INSTALL_DIR "${CMAKE_BINARY_DIR}/taglib"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/taglib"
    CMAKE_CACHE_ARGS "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true"
)

ExternalProject_Add_Step(
    taglib
    forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo_append ""
    COMMENT "Forcing build step for taglib"
    DEPENDEES configure
    DEPENDERS build
    ALWAYS 1
)