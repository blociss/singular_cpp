cmake_minimum_required(VERSION 3.10)

# Find GCC compiler
find_program(GCC_PATH gcc)
find_program(GPP_PATH g++)

if(NOT GCC_PATH OR NOT GPP_PATH)
    message(FATAL_ERROR "gcc/g++ not found. Please install gcc.")
endif()

# Set compiler for all platforms
set(CMAKE_C_COMPILER "${GCC_PATH}")
set(CMAKE_CXX_COMPILER "${GPP_PATH}")

# Set macOS specific settings
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS" FORCE)
    include_directories(SYSTEM ${CMAKE_OSX_SYSROOT}/usr/include/c++/v1)
endif()

project(createring_test VERSION 1.0)

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
add_executable(createring_test 
    createring_test.cpp
)

# Find Singular libraries
file(GLOB SINGULAR_LIBS 
    "${SINGULAR_INSTALL_DIR}/lib/*${LIB_SUFFIX}")

# Link libraries
target_link_libraries(createring_test 
    ${SINGULAR_LIBS} 
    gmp
    flint
    ${PLATFORM_LIBS}
)

# Install rule
install(TARGETS createring_test
    RUNTIME DESTINATION bin
)
