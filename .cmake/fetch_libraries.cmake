include(FetchContent)

# Include taglib
FetchContent_Declare(
    taglib
    GIT_REPOSITORY https://github.com/taglib/taglib
    GIT_TAG v1.12
)

FetchContent_MakeAvailable(taglib)