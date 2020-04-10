
@echo off

mkdir build\mono\js
xcopy /y /E tests\testData build\mono\testData\ 

em++  -Wall -std=c++17  -I core/includes  core/src/main.cpp tools/emscripten/main.cpp -s WASM=1 -o hello.html

REM clang++ -g -O0 -Wall -std=c++17 -o build/mono/Tests.exe -I core/includes tests/src/main.cpp build/mono/core.lib
REM pushd build\mono
REM .\Tests.exe
REM popd



