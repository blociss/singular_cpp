#!/bin/bash

# Set the Singular installation directory
#SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-snapshot_22_03-5jvwtprazqirywu2triw6rprjazzi3so"
SINGULAR_INSTALL_DIR=~/Singular4
FLINT_HOME="/home/atraore/tmp"

# Set the library path (corrected syntax)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib

# Set the DEP_LIBS variable (corrected syntax)
DEP_LIBS=$FLINT_HOME/lib

# Set the library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SINGULAR_INSTALL_DIR/lib

# Navigate to the installation directory
cd /home/atraore/gpi/try_gpi/singular_cpp/install_dir/bin

# Run the executable
./singular_cpp
