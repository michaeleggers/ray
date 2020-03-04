@echo off

mkdir ..\build
pushd ..\build

REM raytracer build
REM cl /EHsc -Zi /FC ..\code\raytracer.cpp user32.lib
REM raytracer release build
REM cl /FC /O2 ..\code\raytracer.cpp user32.lib /Fe"raytracer_release.exe"

clang-cl /Z7 /Wall ..\code\raytracer.cpp -o ray_clang.exe

REM vector test build
REM cl -Zi /FC ..\code\vector_test.cpp user32.lib

popd
