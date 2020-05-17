@echo off

REM setting up the folders and data 
mkdir ..\build\mono
mkdir ..\core\src\autogen
xcopy /y /E ..\tests\testData ..\build\testData\ 

REM build the meta compiler
clang++ -g -O0 -Wall -Wextra -Wpedantic -std=c++17  -Wno-gnu-zero-variadic-macro-arguments -Wno-deprecated-declarations -o ..\build/mono/metacompiler.exe -I ..\core/includes ..\tools/metacompiler/main.cpp
echo -- metacompiler built

REM run the metacompiler
pushd ..\build\mono
.\metacompiler.exe
popd
echo -- metacompiler run

REM build the core library 
clang++ -g -O0 -c -Wall  -Wextra -Wpedantic -std=c++17 -Wno-gnu-zero-variadic-macro-arguments -Wno-deprecated-declarations -o ..\build/mono/core.o -I ..\core/includes ..\core/src/main.cpp
llvm-ar rc ..\build/mono/core.lib ..\build/mono/core.o
echo -- core built

clang++ -g -O0 -Wall -Wextra -Wpedantic -std=c++17 -Wno-gnu-zero-variadic-macro-arguments -Wno-deprecated-declarations -o ..\build/mono/Tests.exe -I ..\core/includes ..\tests/src/main.cpp ..\build/mono/core.lib
echo -- test built
pushd ..\build\mono
.\Tests.exe
popd




