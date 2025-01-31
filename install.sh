#!/bin/bash

# Create necessary directories
mkdir -p ~/gpi/try_gpi/my_singular/build_dir ~/gpi/try_gpi/my_singular/install_dir

# Define variables
INSTALL_PREFIX="/home/atraore/gpi/try_gpi/my_singular/install_dir/"
BUILD_TYPE="Release"
BOOST_NO_CMAKE="on"
BUILD_DIR="/home/atraore/gpi/try_gpi/my_singular/build_dir/"
SOURCE_DIR="/home/atraore/gpi/try_gpi/my_singular/simple_singular"

# Set FLINT home directory
FLINT_HOME="/usr/local/"

# Set GMP home directory
GMP_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-u44gipctyowzsbhpnkn2a6ffddkpfcyk"

# Set Singular installation directory
SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-snapshot_22_03-5jvwtprazqirywu2triw6rprjazzi3so"

# Set the library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib

# Set the DEP_LIBS variable
DEP_LIBS=$FLINT_HOME/lib

# Run CMake
cmake -D CMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
      -D CMAKE_BUILD_TYPE=$BUILD_TYPE \
      -D SINGULAR_HOME=$SINGULAR_INSTALL_DIR \
      -D Boost_NO_BOOST_CMAKE=$BOOST_NO_CMAKE \
      -D GMP_HOME=$GMP_HOME \
      -D FLINT_HOME=$FLINT_HOME \
      -B $BUILD_DIR \
      -S $SOURCE_DIR

# Build and install
cmake --build "$BUILD_DIR" --target install -- -j $(nproc)
