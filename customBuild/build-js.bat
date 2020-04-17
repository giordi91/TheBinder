
@echo off

mkdir build\mono\js

REM copy the rest of the files like js and css out to the build folder
xcopy /y /s /i ..\tools\emscripten\web\css ..\build\mono\js\css\
xcopy /y /s /i ..\tools\emscripten\web\js  ..\build\mono\js\js\

em++ -g0  -Wall -std=c++17  -I ../core/includes  ../core/src/main.cpp ../tools/emscripten/main.cpp  -o ../build\mono\js\bind.html --shell-file ../tools/emscripten/web/wasm.html  -s MINIFY_HTML=0 -s WASM=1   -s EXPORTED_FUNCTIONS=["_bindExecute"] -s EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap","ccall"]  -s NO_EXIT_RUNTIME=1 




