#!/bin/bash
mkdir -p built/lgpl

for i in Mars Venus Pluto Saturn Uranus
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

for i in Earth Mercury
do 
    cd $i
    make -j8
    if [ $? != 0 ]
    then
        echo BUILD FAILED FOR $i
        exit 1
    fi
    cp build/*.bin ../built/lgpl/
    cd ..
done

cd Jupiter/CloudSeed
make -j8
if [ $? != 0 ]
then
    echo BUILD FAILED FOR Jupiter:Cloudseed
    exit 1
fi
cd ../..

cd Jupiter/jupiter
make -j8
if [ $? != 0 ]
then
    echo BUILD FAILED FOR Jupiter
    exit 1
fi
cp build/jupiter.bin ../../built/
cd ../..

cd Neptune/CloudSeed
make -j8
if [ $? != 0 ]
then
    echo BUILD FAILED FOR Neptune:Cloudseed
    exit 1
fi
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
