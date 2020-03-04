@echo off

mkdir ..\build
pushd ..\build

REM raytracer build
REM cl /EHsc -Zi /FC ..\code\raytracer.cpp user32.lib
REM raytracer release build
REM cl /FC /O2 ..\code\raytracer.cpp user32.lib /Fe"raytracer_release.exe"

clang-cl /Z7 /Wall -Wno-c++98-compat -Wno-old-style-cast -Wno-zero-as-null-pointer-constant -Wno-double-promotion ..\code\raytracer.cpp -o ray_clang.exe
REM clang-cl /O2 /Wall -Wno-c++98-compat -Wno-old-style-cast ..\code\raytracer.cpp -o ray_clang_fast.exe

REM vector test build
REM cl -Zi /FC ..\code\vector_test.cpp user32.lib

popd
