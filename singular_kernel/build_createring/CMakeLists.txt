cmake_minimum_required(VERSION 3.10)
project(createring_test)

# Find Singular
set(SINGULAR_INSTALL_DIR $ENV{SINGULAR_INSTALL_DIR})
set(FLINT_HOME $ENV{FLINT_HOME})
set(GMP_HOME $ENV{GMP_HOME})

# Include directories
include_directories(
    ${SINGULAR_INSTALL_DIR}/include/singular
    ${FLINT_HOME}/include
    ${GMP_HOME}/include
)

# Link directories
link_directories(
    ${SINGULAR_INSTALL_DIR}/lib
    ${FLINT_HOME}/lib
    ${GMP_HOME}/lib
)

# Add executable
add_executable(createring_test createring_test.cpp)

# Link libraries
target_link_libraries(createring_test Singular flint gmp)
