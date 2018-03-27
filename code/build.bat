@echo off

mkdir ..\build
pushd ..\build

REM raytracer build
cl -Zi /FC ..\code\raytracer.cpp user32.lib

popd
