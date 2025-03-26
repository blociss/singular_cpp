#!/bin/bash

##################################################
#         Universal GCC + C++17 Installer        #
##################################################

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR"
BUILD_DIR="$SCRIPT_DIR/build_dir"
INSTALL_PREFIX="$SCRIPT_DIR/install_dir"
BUILD_TYPE="Release"
rm -rf "$BUILD_DIR/*" "$INSTALL_PREFIX/*"

mkdir -p "$BUILD_DIR" "$INSTALL_PREFIX"

# Detect OS
OS=$(uname)
echo "[INFO] Detected OS: $OS"

CMAKE_C_FLAGS=""
CMAKE_CXX_FLAGS=""
CC=""
CXX=""

if [ "$OS" = "Darwin" ]; then
    echo "[INFO] Configuring for macOS"

    if ! command -v brew &> /dev/null; then
        echo "[ERROR] Homebrew not found. Please install it first:"
        echo '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
        exit 1
    fi

    for pkg in gcc cmake flint gmp singular; do
        if ! brew list $pkg &> /dev/null; then
            echo "[INFO] Installing $pkg via Homebrew..."
            brew install $pkg
        fi
    done

    GCC_PREFIX=$(brew --prefix gcc)

    # Dynamically detect latest GCC version (e.g., 14)
    GCC_VERSION=$(ls "$GCC_PREFIX/bin" | grep -E '^g\+\+-[0-9]+$' | sed 's/g++-//' | sort -V | tail -n1)
    export CC="$GCC_PREFIX/bin/gcc-$GCC_VERSION"
    export CXX="$GCC_PREFIX/bin/g++-$GCC_VERSION"

    # Verify
    if [ ! -x "$CXX" ]; then
        echo "[ERROR] Could not find working GCC version. Got: $CXX"
        exit 1
    fi

    FLINT_HOME="/opt/homebrew/opt/flint"
    GMP_HOME="/opt/homebrew/opt/gmp"
    SINGULAR_INSTALL_DIR="/opt/homebrew/opt/singular"
    export DYLD_LIBRARY_PATH="$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$DYLD_LIBRARY_PATH"

    PARALLEL_FLAG="-j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
    LIB_PATH_VAR="DYLD_LIBRARY_PATH"

else
    echo "[INFO] Configuring for Linux"

    export CC="/usr/bin/gcc"
    export CXX="/usr/bin/g++"

    FLINT_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/flint-2.6.3-pb3i4qjyjz7pqkpf6cs7wk6ro5pl564i"
    GMP_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-gjqp7e3m3fik4wsuqqcxv2brlj2wkyza"
    SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-4.4.0p2-k7rgdkzo5prqsvxjckejvcdvxgjr64bk"

    export LD_LIBRARY_PATH="$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$LD_LIBRARY_PATH"
    PARALLEL_FLAG="-j$(nproc)"
    LIB_PATH_VAR="LD_LIBRARY_PATH"
fi

export FLINT_HOME
export GMP_HOME
export SINGULAR_INSTALL_DIR

# Show summary
echo "[INFO] FLINT_HOME: $FLINT_HOME"
echo "[INFO] GMP_HOME: $GMP_HOME"
echo "[INFO] SINGULAR_INSTALL_DIR: $SINGULAR_INSTALL_DIR"
echo "[INFO] $LIB_PATH_VAR: ${!LIB_PATH_VAR}"
echo "[INFO] Using compiler: $CXX"

# Clean build
rm -rf "$BUILD_DIR" "$INSTALL_PREFIX"
mkdir -p "$BUILD_DIR" "$INSTALL_PREFIX"

# Run CMake with C++17 and compiler
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
    -D CMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -D CMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -D CMAKE_C_COMPILER="$CC" \
    -D CMAKE_CXX_COMPILER="$CXX" \
    -D CMAKE_CXX_STANDARD=17 \
    -D CMAKE_CXX_STANDARD_REQUIRED=ON

# Build
cmake --build "$BUILD_DIR" --target install -- $PARALLEL_FLAG

# Done
echo "✅ Build complete. Binary is in: $INSTALL_PREFIX/bin"
