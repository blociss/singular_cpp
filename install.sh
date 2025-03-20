#!/bin/bash
##################################################
#     Replace all paths with yours               #
#     Singular path and flint path               #
##################################################
# Create necessary directories
#rm -rf ~/gpi/try_gpi/singular_cpp/build_dir/* ~/gpi/try_gpi/singular_cpp/install_dir/*

mkdir -p ~/gpi/try_gpi/singular_cpp/build_dir ~/gpi/try_gpi/singular_cpp/install_dir

# Define variables
INSTALL_PREFIX="/home/atraore/gpi/try_gpi/singular_cpp/install_dir/" 
BUILD_TYPE="Release"
BOOST_NO_CMAKE="on"
BUILD_DIR="/home/atraore/gpi/try_gpi/singular_cpp/build_dir/"
SOURCE_DIR="/home/atraore/gpi/try_gpi/singular_cpp/"

# Set FLINT home directory
FLINT_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/flint-2.6.3-pb3i4qjyjz7pqkpf6cs7wk6ro5pl564i"

# Set GMP home directory
GMP_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-gjqp7e3m3fik4wsuqqcxv2brlj2wkyza"

# Set Singular install
SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-4.4.0p2-k7rgdkzo5prqsvxjckejvcdvxgjr64bk"

# Set the library path correctly
export LD_LIBRARY_PATH=$GPISPACE_ROOT/lib:$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$LD_LIBRARY_PATH

# Debug statements to verify the environment variables
echo "SINGULAR_INSTALL_DIR: $SINGULAR_INSTALL_DIR"
echo "FLINT_HOME: $FLINT_HOME"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"



# Run CMake
cmake -D CMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
      -D CMAKE_BUILD_TYPE=$BUILD_TYPE \
      -D Boost_NO_BOOST_CMAKE=$BOOST_NO_CMAKE \
      -D FLINT_HOME=$FLINT_HOME \
      -B $BUILD_DIR \
      -S $SOURCE_DIR

# Build and install
cmake --build "$BUILD_DIR" --target install -- -j $(nproc)
