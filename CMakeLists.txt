cmake_minimum_required(VERSION 3.5)
project(ciderutils C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

# Windows
if(MSVC)
    include_directories("extern/dirent_win32") # dirent for Windows

    if(CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
        # Generate node.lib
        execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

include(fetch_libraries)

set(SOURCE_FILES
    src/interop.c
    src/utils.cc # Unfortunately, C++ is required for taglib
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_JS_INC})
target_include_directories(${PROJECT_NAME} PRIVATE "source")

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
add_compile_definitions(NAPI_VERSION=3)

target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_JS_LIB} zlibstatic)
target_link_libraries(${PROJECT_NAME} PRIVATE tag)