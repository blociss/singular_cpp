#!/bin/bash

##################################################
#          FLINT CMake Build Script              #
#        Run from within the flint dir           #
##################################################
#rm -rf ~/gpi/try_gpi/singular_cpp/build_dir/* ~/gpi/try_gpi/singular_cpp/install_dir/*

mkdir -p ~/gpi/try_gpi/singular_cpp/flint/build_dir ~/gpi/try_gpi/singular_cpp/flint/install_dir
# Define base directories
PROJECT_ROOT="/home/atraore/gpi/try_gpi/singular_cpp/flint"
BUILD_DIR="${PROJECT_ROOT}/build_dir"
INSTALL_PREFIX="${PROJECT_ROOT}/install_dir"
SOURCE_DIR="${PROJECT_ROOT}"
BUILD_TYPE="Release"

# FLINT and GMP paths
FLINT_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/flint-2.6.3-pb3i4qjyjz7pqkpf6cs7wk6ro5pl564i"
GMP_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-gjqp7e3m3fik4wsuqqcxv2brlj2wkyza"

# Create directories
mkdir -p "$BUILD_DIR" "$INSTALL_PREFIX"

# Export runtime lib path
export LD_LIBRARY_PATH="${FLINT_HOME}/lib:${GMP_HOME}/lib:${LD_LIBRARY_PATH}"

echo "[INFO] FLINT_HOME: $FLINT_HOME"
echo "[INFO] GMP_HOME: $GMP_HOME"
echo "[INFO] LD_LIBRARY_PATH: $LD_LIBRARY_PATH"

# Configure
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
    -D CMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -D CMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -D FLINT_HOME="$FLINT_HOME" \
    -D GMP_HOME="$GMP_HOME"

# Build & install
cmake --build "$BUILD_DIR" --target install -- -j$(nproc)

echo "✅ Build complete. Run with: $INSTALL_PREFIX/bin/gauss"
