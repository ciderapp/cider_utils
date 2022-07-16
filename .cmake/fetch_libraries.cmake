include(FetchContent)

# Include zlib-ng
set(ZLIB_ENABLE_TESTS OFF)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng
    GIT_TAG 2.0.6
)

# Include taglib
# set(ZLIB_SOURCE ON)
FetchContent_Declare(
    taglib
    GIT_REPOSITORY https://github.com/taglib/taglib
    GIT_TAG v1.12
)

FetchContent_MakeAvailable(zlib taglib)