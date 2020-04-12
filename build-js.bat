
@echo off

mkdir build\mono\js

em++  -g0 -Wall -std=c++17  -I core/includes  core/src/main.cpp tools/emscripten/main.cpp -s WASM=1   -s EXPORTED_FUNCTIONS=["_main","_test"] -o build\mono\js\bind4.html  --shell-file wasm.html -s MINIFY_HTML=0 -s EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap","ccall"] 

REM clang++ -g -O0 -Wall -std=c++17 -o build/mono/Tests.exe -I core/includes tests/src/main.cpp build/mono/core.lib
REM pushd build\mono
REM .\Tests.exe
REM popd



