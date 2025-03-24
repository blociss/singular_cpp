cmake_minimum_required(VERSION 3.10)
project(createring_test)

# Find Singular
if(NOT DEFINED ENV{SINGULAR_INSTALL_DIR})
    message(FATAL_ERROR "SINGULAR_INSTALL_DIR environment variable not set")
endif()

set(SINGULAR_DIR $ENV{SINGULAR_INSTALL_DIR})
include_directories(${SINGULAR_DIR}/include ${SINGULAR_DIR}/include/singular)
link_directories(${SINGULAR_DIR}/lib)

# Add executable
add_executable(createring_test createring.cpp)

# Link against Singular library
target_link_libraries(createring_test Singular polys-4.4.0 omalloc-0.9.6)
