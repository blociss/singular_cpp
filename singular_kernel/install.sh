#!/bin/bash
##################################################
#     Set all paths properly before building     #
##################################################
rm -rf /home/atraore/gpi/try_gpi/singular_cpp/singular_kernel/build_dir/*

# Base project path
BASE_DIR="/home/atraore/gpi/try_gpi/singular_cpp/singular_kernel"

# Create necessary directories
mkdir -p "$BASE_DIR/build_dir" "$BASE_DIR/install_dir"

# Variables
SOURCE_DIR="$BASE_DIR"
BUILD_DIR="$BASE_DIR/build_dir"
INSTALL_PREFIX="$BASE_DIR/install_dir"
BUILD_TYPE="Release"

# External dependencies
FLINT_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/flint-2.6.3-pb3i4qjyjz7pqkpf6cs7wk6ro5pl564i"
GMP_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-gjqp7e3m3fik4wsuqqcxv2brlj2wkyza"
SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-4.4.0p2-k7rgdkzo5prqsvxjckejvcdvxgjr64bk"

# Export for use in CMake
export FLINT_HOME
export GMP_HOME
export SINGULAR_INSTALL_DIR

# Extend LD_LIBRARY_PATH
export LD_LIBRARY_PATH="$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$LD_LIBRARY_PATH"

# Info
echo "[INFO] FLINT_HOME: $FLINT_HOME"
echo "[INFO] GMP_HOME: $GMP_HOME"
echo "[INFO] SINGULAR_INSTALL_DIR: $SINGULAR_INSTALL_DIR"
echo "[INFO] LD_LIBRARY_PATH: $LD_LIBRARY_PATH"

# Run CMake
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
      -D CMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
      -D CMAKE_BUILD_TYPE="$BUILD_TYPE"

# Build and install
cmake --build "$BUILD_DIR" --target install -- -j $(nproc)

echo "✅ Build complete. Binary is in: $INSTALL_PREFIX/bin"
