cmake_minimum_required(VERSION 3.15)
project(serin VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create library
add_library(libtoon STATIC
    src/serin.cpp
)

# Include directories
target_include_directories(libtoon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>
    $<INSTALL_INTERFACE:includes>
)

# Compiler options
target_compile_options(libtoon PRIVATE
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
        target_link_libraries(${EXAMPLE_NAME} PUBLIC libtoon)
    endforeach()
endif()

# Build CLI tool
add_executable(toon cli.cpp)
target_link_libraries(toon PUBLIC libtoon)

# Install targets
install(TARGETS libtoon
    EXPORT libtoonTargets
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
install(EXPORT libtoonTargets
    FILE libtoonTargets.cmake
    NAMESPACE libtoon::
    DESTINATION lib/cmake/libtoon
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libtoonConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/libtoonConfigVersion.cmake"
    DESTINATION lib/cmake/libtoon
)
