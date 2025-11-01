cmake_minimum_required(VERSION 3.15)

option(BUILD_PYTHON "export python module" off)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ version selection")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(PyProject)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)



project(${PyProject_NAME} LANGUAGES CXX C VERSION ${PyProject_VERSION})



file(GLOB SERIN_SOURCES_C ${PROJECT_SOURCE_DIR}/src/sources/*.c)
file(GLOB SERIN_SOURCES_CPP ${PROJECT_SOURCE_DIR}/src/sources/*.cpp)
set(SERIN_SOURCES ${SERIN_SOURCES_CPP} ${SERIN_SOURCES_C})

# Create library
add_library(serin ${SERIN_SOURCES})

# Include directories
target_include_directories(serin
    PUBLIC ${PROJECT_SOURCE_DIR}/includes
    PRIVATE ${PROJECT_SOURCE_DIR}/src/sources
)

# Compiler options
target_compile_options(serin PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Build examples
option(BUILD_EXAMPLES "Build example programs" ON)


include(SetupPython)

if (BUILD_EXAMPLES)
    message(STATUS "Building examples")

    # Build all examples using glob pattern
    file(GLOB EXAMPLE_SOURCES ${PROJECT_SOURCE_DIR}/examples/*.cpp)
    foreach(EXAMPLE_SOURCE IN LISTS EXAMPLE_SOURCES)
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        target_link_libraries(${EXAMPLE_NAME} PUBLIC serin)
    endforeach()
endif()

# Build CLI tool
add_executable(serin-cli src/sources/cli.cpp)
target_link_libraries(serin-cli PUBLIC serin)
set_target_properties(serin-cli PROPERTIES OUTPUT_NAME serin-cli)
