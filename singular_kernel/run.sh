#!/bin/bash

# Get the directory containing this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Detect OS
OS=$(uname)

# Set library path based on OS
if [ "$OS" = "Darwin" ]; then
    # macOS settings
    FLINT_HOME="/opt/homebrew/opt/flint"
    GMP_HOME="/opt/homebrew/opt/gmp"
    SINGULAR_INSTALL_DIR="/opt/homebrew/opt/singular"
    export DYLD_LIBRARY_PATH="$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$DYLD_LIBRARY_PATH"
else
    # Linux settings
    FLINT_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/flint-2.6.3-pb3i4qjyjz7pqkpf6cs7wk6ro5pl564i"
    GMP_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-gjqp7e3m3fik4wsuqqcxv2brlj2wkyza"
    SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-4.4.0p2-k7rgdkzo5prqsvxjckejvcdvxgjr64bk"
    export LD_LIBRARY_PATH="$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$LD_LIBRARY_PATH"
fi

# Export variables for use in the program
export FLINT_HOME
export GMP_HOME
export SINGULAR_INSTALL_DIR

# Check if binary exists
BINARY="$SCRIPT_DIR/install_dir/bin/singular_cpp"
if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found at $BINARY"
    echo "Please run ./install.sh first"
    exit 1
fi

# Run the executable
"$BINARY"
