@echo off

mkdir build\mono
xcopy /y /E tests\testData build\testData\ 

clang++  -c -Wall -std=c++17 -o build/mono/core.o -I core/includes core/src/main.cpp
llvm-ar rc build/mono/core.lib build/mono/core.o

clang++ -Wall -std=c++17 -o build/mono/Tests.exe -I core/includes tests/src/main.cpp build/mono/core.lib
pushd build\mono
.\Tests.exe
popd




