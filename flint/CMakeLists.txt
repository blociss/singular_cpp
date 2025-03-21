cmake_minimum_required(VERSION 3.10)
project(gauss_flint_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# These will be passed from install.sh
set(FLINT_HOME $ENV{FLINT_HOME})
set(GMP_HOME $ENV{GMP_HOME})

# Include and link paths
include_directories(${FLINT_HOME}/include ${GMP_HOME}/include)
link_directories(${FLINT_HOME}/lib ${GMP_HOME}/lib)

# Source file is in src/
add_executable(gauss src/gauss.cpp)

# Link against FLINT and GMP
target_link_libraries(gauss flint gmp)

# Install the executable to bin/
install(TARGETS gauss RUNTIME DESTINATION bin)
