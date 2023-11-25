$directoryPath = "./libs/raylib/include/"

New-Item -ItemType Directory -Force -Path $directoryPath

$filesToCopy = @("./raylib/src/raylib.h", "./raylib/src/raymath.h", "./raylib/src/rlgl.h")

foreach ($file in $filesToCopy) {
    Copy-Item -Path $file -Destination $directoryPath -Force
}

Set-Location -Path "./raylib/src/"
Invoke-Expression "make PLATFORM=PLATFORM_DESKTOP"

Set-Location -Path "../../"

# rcore.o rshapes.o rtextures.o rtext.o utils.o rglfw.o rmodels.o raudio.o librarylib.a
$filesToCopy = @("./raylib/src/rcore.o", "./raylib/src/rshapes.o", "./raylib/src/rtextures.o", "./raylib/src/rtext.o", "./raylib/src/utils.o", "./raylib/src/rglfw.o", "./raylib/src/rmodels.o", "./raylib/src/raudio.o", "./raylib/src/libraylib.a")

foreach ($file in $filesToCopy) {
    Copy-Item -Path $file -Destination "./libs/raylib/" -Force
}
