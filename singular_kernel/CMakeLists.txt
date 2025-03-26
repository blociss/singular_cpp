cmake_minimum_required(VERSION 3.10)
project(singular_cpp VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Get environment paths
if(NOT DEFINED ENV{SINGULAR_INSTALL_DIR})
    message(FATAL_ERROR "SINGULAR_INSTALL_DIR environment variable not set")
endif()
if(NOT DEFINED ENV{GMP_HOME})
    message(FATAL_ERROR "GMP_HOME environment variable not set")
endif()
if(NOT DEFINED ENV{FLINT_HOME})
    message(FATAL_ERROR "FLINT_HOME environment variable not set")
endif()

set(SINGULAR_INSTALL_DIR "$ENV{SINGULAR_INSTALL_DIR}")
set(GMP_HOME "$ENV{GMP_HOME}")
set(FLINT_HOME "$ENV{FLINT_HOME}")

# Platform-specific
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(LIB_SUFFIX ".dylib")
    set(PLATFORM_LIBS "")
else()
    set(LIB_SUFFIX ".so")
    set(PLATFORM_LIBS pthread dl m)
endif()

# Include paths
include_directories(
    ${SINGULAR_INSTALL_DIR}/include
    ${SINGULAR_INSTALL_DIR}/include/singular
    ${GMP_HOME}/include
    ${FLINT_HOME}/include
)

link_directories(
    ${SINGULAR_INSTALL_DIR}/lib
    ${GMP_HOME}/lib
    ${FLINT_HOME}/lib
)

# Add executable
add_executable(singular_cpp 
    src/main.cpp
    src/feynman.cpp
    src/feynman.h
)

# Link
file(GLOB SINGULAR_LIBS "${SINGULAR_INSTALL_DIR}/lib/*${LIB_SUFFIX}")
target_link_libraries(singular_cpp 
    ${SINGULAR_LIBS}
    gmp
    flint
    ${PLATFORM_LIBS}
)

# Install
install(TARGETS singular_cpp RUNTIME DESTINATION bin)
