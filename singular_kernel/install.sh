#!/bin/bash

##################################################
#     Set all paths properly before building     #
##################################################

# Get the directory containing this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Create necessary directories
mkdir -p "$SCRIPT_DIR/build_dir" "$SCRIPT_DIR/install_dir"

# Variables
SOURCE_DIR="$SCRIPT_DIR"
BUILD_DIR="$SCRIPT_DIR/build_dir"
INSTALL_PREFIX="$SCRIPT_DIR/install_dir"
BUILD_TYPE="Release"

# Detect OS
OS=$(uname)
echo "[INFO] Detected OS: $OS"

# Initialize compiler and flags
CMAKE_C_FLAGS=""
CMAKE_CXX_FLAGS=""

# Set platform-specific variables
if [ "$OS" = "Darwin" ]; then
    echo "[INFO] Configuring for macOS"

    # Check for Xcode Command Line Tools
    if ! command -v xcode-select &> /dev/null; then
        echo "[ERROR] Xcode Command Line Tools not found. Installing..."
        xcode-select --install
        echo "[ERROR] Please run this script again after installation completes"
        exit 1
    fi

    # Check for Homebrew
    if ! command -v brew &> /dev/null; then
        echo "[ERROR] Homebrew not found. Please install Homebrew first:"
        echo '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
        exit 1
    fi

    # Check required packages
    for pkg in cmake flint gmp singular; do
        if ! brew list $pkg &> /dev/null; then
            echo "[INFO] Installing $pkg via Homebrew..."
            brew install $pkg
        fi
    done

    # Paths for macOS M1 (Homebrew default)
    FLINT_HOME="/opt/homebrew/opt/flint"
    GMP_HOME="/opt/homebrew/opt/gmp"
    SINGULAR_INSTALL_DIR="/opt/homebrew/opt/singular"
    export DYLD_LIBRARY_PATH="$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$DYLD_LIBRARY_PATH"

    # macOS SDK + libc++ headers
    export SDKROOT=$(xcrun --show-sdk-path)
    CLT_CPP_INCLUDE="/Library/Developer/CommandLineTools/usr/include/c++/v1"
    CMAKE_C_FLAGS="-isysroot $SDKROOT"
    CMAKE_CXX_FLAGS="-isysroot $SDKROOT -I$CLT_CPP_INCLUDE"

    PARALLEL_FLAG="-j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
    LIB_PATH_VAR="DYLD_LIBRARY_PATH"

else
    echo "[INFO] Configuring for Linux"

    # Ensure standard dev packages are installed
    if ! dpkg -s build-essential &> /dev/null; then
        echo "[WARNING] build-essential not found. Consider running:"
        echo "  sudo apt update && sudo apt install build-essential"
    fi

    # Linux (Spack paths — update if needed)
    FLINT_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/flint-2.6.3-pb3i4qjyjz7pqkpf6cs7wk6ro5pl564i"
    GMP_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-gjqp7e3m3fik4wsuqqcxv2brlj2wkyza"
    SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-4.4.0p2-k7rgdkzo5prqsvxjckejvcdvxgjr64bk"
    export LD_LIBRARY_PATH="$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$LD_LIBRARY_PATH"

    PARALLEL_FLAG="-j$(nproc)"
    LIB_PATH_VAR="LD_LIBRARY_PATH"
fi

# Export library paths for runtime and CMake
export FLINT_HOME
export GMP_HOME
export SINGULAR_INSTALL_DIR

# Print config
echo "[INFO] FLINT_HOME: $FLINT_HOME"
echo "[INFO] GMP_HOME: $GMP_HOME"
echo "[INFO] SINGULAR_INSTALL_DIR: $SINGULAR_INSTALL_DIR"
echo "[INFO] $LIB_PATH_VAR: ${!LIB_PATH_VAR}"
echo "[INFO] CMAKE_C_FLAGS: $CMAKE_C_FLAGS"
echo "[INFO] CMAKE_CXX_FLAGS: $CMAKE_CXX_FLAGS"

# Clean and create build dir
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure with CMake
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
    -D CMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -D CMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -D CMAKE_C_COMPILER=clang \
    -D CMAKE_CXX_COMPILER=clang++ \
    -D CMAKE_C_FLAGS="$CMAKE_C_FLAGS" \
    -D CMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS"

# Build & install
cmake --build "$BUILD_DIR" --target install -- $PARALLEL_FLAG

# Final message
echo "✅ Build complete. Binary is in: $INSTALL_PREFIX/bin"
