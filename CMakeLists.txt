cmake_minimum_required(VERSION 3.5)
project(ciderutils C CXX)

# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_SHARED_LIBS "" off)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-z,defs")

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
    src/b64.c

    src/utils.cc # Unfortunately, C++ is required for taglib
)

link_directories(${CMAKE_BINARY_DIR}/taglib/lib)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_JS_INC} ${CMAKE_BINARY_DIR}/taglib/include)
target_include_directories(${PROJECT_NAME} PRIVATE "source")

add_dependencies(${PROJECT_NAME} taglib)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
add_compile_definitions(NAPI_VERSION=3)

target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_JS_LIB})
target_link_libraries(${PROJECT_NAME} PRIVATE zlibstatic tag)