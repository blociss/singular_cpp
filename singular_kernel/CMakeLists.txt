cmake_minimum_required(VERSION 3.10)
project(singular_cpp VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pick up Singular install from environment
set(SINGULAR_INSTALL_DIR "$ENV{SINGULAR_INSTALL_DIR}")
set(GMP_HOME "$ENV{GMP_HOME}")

# Header paths
include_directories(${SINGULAR_INSTALL_DIR}/include)
include_directories(${SINGULAR_INSTALL_DIR}/include/singular)
include_directories(${GMP_HOME}/include)

# Link directories
link_directories(${SINGULAR_INSTALL_DIR}/lib)
link_directories(${GMP_HOME}/lib)

# Add executable from src/
add_executable(singular_cpp 
    src/main.cpp
    src/feynman.cpp
    src/feynman.h)

# Find all shared Singular libs
file(GLOB SINGULAR_LIBS "${SINGULAR_INSTALL_DIR}/lib/*.so")

# Link libraries
target_link_libraries(singular_cpp ${SINGULAR_LIBS} gmp pthread dl m)

# Install rule
install(TARGETS singular_cpp
        RUNTIME DESTINATION bin)
