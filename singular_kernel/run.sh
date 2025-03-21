#!/bin/bash

# Set the Singular installation directory
SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-4.4.0p2-4bgipnkqrvp6anrqew3a5a46go5vj7kh"
FLINT_HOME="/home/atraore/tmp"

# Set the library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib

# Set the DEP_LIBS variable
DEP_LIBS=$FLINT_HOME/lib

# Navigate to the installation directory
cd /home/atraore/gpi/try_gpi/singular_cpp/singular_kernel/install_dir/bin

# Run the executable with Valgrind
#valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./singular_cpp
./singular_cpp
#gdb ./singular_cpp
