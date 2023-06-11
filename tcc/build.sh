#!/bin/sh

mkdir -p build && cd build
cmake ..
make
sudo make install
cd ..

# build the samples

mkdir -p build/samples

samples="chartest.c comptest.c externs.c extstr.c exttest.c fact.c floattest.c"\
" floattest2.c fread.c fwrite.c notify.c or_equals.c primes.c sort.c strtest.c"\
" switchtest.c system.c test1.c test2.c timer.c"

for sample in $samples
do
    sample=`basename $sample .c`
    echo $sample
    tcc test/$sample.c > build/samples/$sample.v
    vasm build/samples/$sample.v -o build/samples/$sample
done
