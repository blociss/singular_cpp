#!/bin/bash

# Set the Singular installation directory
SINGULAR_INSTALL_DIR="/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-snapshot_22_03-5jvwtprazqirywu2triw6rprjazzi3so"

# Set the library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SINGULAR_INSTALL_DIR/lib

# Navigate to the installation directory
cd /home/atraore/gpi/try_gpi/my_singular/install_dir/bin

# Run the executable
./simple_singular
