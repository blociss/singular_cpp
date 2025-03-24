cmake_minimum_required(VERSION 3.10)

# Set macOS deployment target and architecture
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS" FORCE)
    include_directories(SYSTEM ${CMAKE_OSX_SYSROOT}/usr/include/c++/v1)
    # Find Xcode command line tools path
    execute_process(
        COMMAND xcode-select -p
        OUTPUT_VARIABLE XCODE_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(XCODE_PATH)
        set(CMAKE_C_COMPILER "${XCODE_PATH}/usr/bin/clang")
        set(CMAKE_CXX_COMPILER "${XCODE_PATH}/usr/bin/clang++")
    else()
        set(CMAKE_C_COMPILER "/usr/bin/clang")
        set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
    endif()
else()
    # Linux settings
    set(CMAKE_C_COMPILER "/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER "/usr/bin/g++")
endif()

project(singular_cpp VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS specific compiler settings
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

# Check required environment variables
if(NOT DEFINED ENV{SINGULAR_INSTALL_DIR})
    message(FATAL_ERROR "SINGULAR_INSTALL_DIR environment variable not set")
endif()
if(NOT DEFINED ENV{GMP_HOME})
    message(FATAL_ERROR "GMP_HOME environment variable not set")
endif()
if(NOT DEFINED ENV{FLINT_HOME})
    message(FATAL_ERROR "FLINT_HOME environment variable not set")
endif()

# Set paths from environment
set(SINGULAR_INSTALL_DIR "$ENV{SINGULAR_INSTALL_DIR}")
set(GMP_HOME "$ENV{GMP_HOME}")
set(FLINT_HOME "$ENV{FLINT_HOME}")

# Platform-specific settings
if(APPLE)
    # macOS settings
    set(CMAKE_MACOSX_RPATH ON)
    set(LIB_SUFFIX ".dylib")
    set(PLATFORM_LIBS "")
    # Handle M1 architecture
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    endif()
    # Add homebrew lib paths for macOS
    include_directories("/opt/homebrew/include")
    link_directories("/opt/homebrew/lib")
else()
    # Linux settings
    set(LIB_SUFFIX ".so")
    set(PLATFORM_LIBS pthread dl m)
endif()

# Header paths
include_directories(
    ${SINGULAR_INSTALL_DIR}/include
    ${SINGULAR_INSTALL_DIR}/include/singular
    ${GMP_HOME}/include
    ${FLINT_HOME}/include
)

# Link directories
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

# Find Singular libraries
file(GLOB SINGULAR_LIBS 
    "${SINGULAR_INSTALL_DIR}/lib/*${LIB_SUFFIX}")

# Link libraries
target_link_libraries(singular_cpp 
    ${SINGULAR_LIBS} 
    gmp
    flint
    ${PLATFORM_LIBS}
)

# Install rule
install(TARGETS singular_cpp
    RUNTIME DESTINATION bin
)
