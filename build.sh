#!/bin/sh

# get base directory
basedir=`pwd`

################################################################################
# Build TGP components
################################################################################
components="libvmasm libvarvm libvmcore tcc vasm vexe vm"

for component in $components
do
    cd $component
    ./build.sh
    cd $basedir
done
