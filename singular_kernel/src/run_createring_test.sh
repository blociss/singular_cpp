#!/bin/bash

# Set environment variables
export SINGULAR_INSTALL_DIR=/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-4.4.0p2-k7rgdkzo5prqsvxjckejvcdvxgjr64bk
export FLINT_HOME=/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/flint-2.6.3-pb3i4qjyjz7pqkpf6cs7wk6ro5pl564i
export GMP_HOME=/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/gmp-6.2.1-gjqp7e3m3fik4wsuqqcxv2brlj2wkyza

# Set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$FLINT_HOME/lib:$GMP_HOME/lib:$SINGULAR_INSTALL_DIR/lib:$LD_LIBRARY_PATH

# Create build directory
mkdir -p build_createring
cd build_createring

# Build
cmake -DCMAKE_BUILD_TYPE=Release -S .. -B . -DCMAKE_INSTALL_PREFIX=../install_dir_createring
make

# Run the test
./createring_test
