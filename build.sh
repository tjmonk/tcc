#!/bin/sh

components="libvmcore libvmasm libvarvm vm vasm vexe"

for component in $components
do
    echo "======================="
    echo "Building $component"
    echo "======================="
    cd $component
    ./build.sh
    cd ..
done
