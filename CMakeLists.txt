cmake_minimum_required(VERSION 3.15)
project(ctoon VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create library
add_library(ctoon STATIC
    src/toon.cpp
)

# Include directories
target_include_directories(ctoon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>
    $<INSTALL_INTERFACE:includes>
)

# Compiler options
target_compile_options(ctoon PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Build examples
option(BUILD_EXAMPLES "Build example programs" ON)

if (BUILD_EXAMPLES)
    message(STATUS "Building examples")

    # Build all examples using glob pattern
    file(GLOB EXAMPLE_SOURCES ${PROJECT_SOURCE_DIR}/examples/*.cpp)
    foreach(EXAMPLE_SOURCE IN LISTS EXAMPLE_SOURCES)
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        target_link_libraries(${EXAMPLE_NAME} PUBLIC ctoon)
    endforeach()
endif()

# Install targets
install(TARGETS ctoon
    EXPORT ctoonTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY includes/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Export package
install(EXPORT ctoonTargets
    FILE ctoonTargets.cmake
    NAMESPACE ctoon::
    DESTINATION lib/cmake/ctoon
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ctoonConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ctoonConfigVersion.cmake"
    DESTINATION lib/cmake/ctoon
)
