cmake_minimum_required(VERSION 3.5)
project(ciderutils)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Windows
if(MSVC)
    target_include_directories(ciderutils PRIVATE "extern/dirent_win32") # dirent for Windows

    if(CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
        # Generate node.lib
        execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
    endif()
endif()

include(fetch_libraries)

set(SOURCE_FILES
    src/interop.c
    src/utils.c
)

add_library(ciderutils SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
target_include_directories(ciderutils PUBLIC ${CMAKE_JS_INC})
target_include_directories(ciderutils PRIVATE "source")

set_target_properties(ciderutils PROPERTIES PREFIX "" SUFFIX ".node")
add_compile_definitions(NAPI_VERSION=${napi_build_version})

target_link_libraries(ciderutils ${CMAKE_JS_LIB} tag_c)