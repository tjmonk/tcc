#!/bin/sh

mkdir -p build && cd build
cmake ..
make
sudo make install
cd ..

# assemble the test programs
cd test
mkdir -p build
files=`ls *.v`
for file in $files
do
    outfile=`basename $file .v`
    vasm $file -o build/$outfile
done
cd ..
