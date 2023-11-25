#!/bin/bash
directoryPath="./libs/raylib/include/"

mkdir -p $directoryPath

filesToCopy=("./raylib/src/raylib.h" "./raylib/src/raymath.h" "./raylib/src/rlgl.h")

for file in "${filesToCopy[@]}"; do
    cp $file $directoryPath
done

cd "./raylib/src/"

make PLATFORM=PLATFORM_DESKTOP

cd "../../"

# rcore.o rshapes.o rtextures.o rtext.o utils.o rglfw.o rmodels.o raudio.o librarylib.a
filesToCopy=("./raylib/src/rcore.o" "./raylib/src/rshapes.o" "./raylib/src/rtextures.o" "./raylib/src/rtext.o" "./raylib/src/utils.o" "./raylib/src/rglfw.o" "./raylib/src/rmodels.o" "./raylib/src/raudio.o" "./raylib/src/libraylib.a")

for file in "${filesToCopy[@]}"; do
    cp $file "./libs/raylib/"
done
