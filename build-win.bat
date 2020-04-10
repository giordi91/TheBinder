@echo off

mkdir build\mono
xcopy /y /E tests\testData build\testData\ 

clang++ -g -O0 -c -Wall -std=c++17 -Wno-deprecated-declarations -o build/mono/core.o -I core/includes core/src/main.cpp
llvm-ar rc build/mono/core.lib build/mono/core.o

clang++ -g -O0 -Wall -std=c++17 -Wno-deprecated-declarations -o build/mono/Tests.exe -I core/includes tests/src/main.cpp build/mono/core.lib
pushd build\mono
.\Tests.exe
popd




