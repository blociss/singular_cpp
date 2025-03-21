#!/bin/bash

# Set the Singular installation directory
SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-4.4.0p2-4bgipnkqrvp6anrqew3a5a46go5vj7kh"

# Set the FLINT and GMP directories
FLINT_HOME="/home/atraore/tmp"
GMP_HOME="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-gjqp7e3m3fik4wsuqqcxv2brlj2wkyza"

# Set the library path
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib"

# Go to install directory
cd /home/atraore/gpi/try_gpi/singular_cpp/flint/install_dir/bin || exit 1

# Run the program
./gauss

