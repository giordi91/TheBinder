#!/bin/bash
mkdir -p ../build/mono/js


cp -r ../tools/emscripten/web/css ../build/mono/js/css/
cp -r ../tools/emscripten/web/js  ../build/mono/js/js/

em++ -g0  -Wall -std=c++17  -I ../core/includes  ../core/src/main.cpp ../tools/emscripten/main.cpp  -o ../build/mono/js/bind.html --shell-file ../tools/emscripten/web/wasm.html  -s MINIFY_HTML=0 -s WASM=1   -s EXPORTED_FUNCTIONS=["_bindExecute"] -s EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap","ccall"]  -s NO_EXIT_RUNTIME=1 -s DISABLE_EXCEPTION_CATCHING=2  -s ALLOW_MEMORY_GROWTH=1
