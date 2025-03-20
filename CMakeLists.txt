cmake_minimum_required(VERSION 3.10)
project(singular_cpp VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Manually set Singular paths
set(SINGULAR_INCLUDE_DIRS "$ENV{SINGULAR_INSTALL_DIR}/include")
set(SINGULAR_LIBRARY_DIR "$ENV{SINGULAR_INSTALL_DIR}/lib")

# Include Singular headers and configuration headers
include_directories(${SINGULAR_INCLUDE_DIRS})
include_directories(${SINGULAR_INCLUDE_DIRS}/singular)

# Add executable
add_executable(singular_cpp src/main.cpp src/singular_functions.cpp)


# Link all Singular libraries
file(GLOB SINGULAR_LIBRARIES "${SINGULAR_LIBRARY_DIR}/*.so")
target_link_libraries(singular_cpp ${SINGULAR_LIBRARIES})
target_link_libraries(singular_cpp -lpthread -ldl -lm)

# Add GMP library to the link command
target_link_libraries(singular_cpp ${SINGULAR_LIBRARIES} gmp -lpthread -ldl -lm)

# Install the executable
install(TARGETS singular_cpp
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)