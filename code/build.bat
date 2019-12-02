@echo off

mkdir ..\build
pushd ..\build

REM raytracer build
cl -Zi /FC ..\code\raytracer.cpp user32.lib

REM raytracer release build
cl /FC /O2 ..\code\raytracer.cpp user32.lib /Fe"raytracer_release.exe"

REM vector test build
REM cl -Zi /FC ..\code\vector_test.cpp user32.lib

popd
