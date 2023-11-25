$directoryPath = "./libs/raylib/include/"

New-Item -ItemType Directory -Force -Path $directoryPath

$filesToCopy = @("./raylib/src/raylib.h", "./raylib/src/raymath.h", "./raylib/src/rlgl.h")

foreach ($file in $filesToCopy) {
    Copy-Item -Path $file -Destination $directoryPath -Force
}

Set-Location -Path "./raylib/src/"
Invoke-Expression "make PLATFORM=PLATFORM_DESKTOP"

Set-Location -Path "../../"

# librarylib.a
$filesToCopy = @("./raylib/src/libraylib.a")

foreach ($file in $filesToCopy) {
    Copy-Item -Path $file -Destination "./libs/raylib/" -Force
}
