#!/bin/bash
for i in Mars Earth Mercury Venus Pluto Saturn Uranus
do 
    cd $i
    make -j8
    if [ $? != 0 ]
    then
        echo BUILD FAILED FOR $i
        exit 1
    fi
    cp build/*.bin ../built/
    cd ..
done

cd Jupiter/jupiter
make -j8
if [ $? != 0 ]
then
    echo BUILD FAILED FOR Jupiter
    exit 1
fi
cp build/jupiter.bin ../../built/
cd ../..

cd Neptune/neptune
make -j8
if [ $? != 0 ]
then
    echo BUILD FAILED FOR Neptune
    exit 1
fi
cp build/neptune.bin ../../built/
cd ../..
