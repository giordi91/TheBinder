# TheBinder 
Master:  ![build-windows](https://github.com/giordi91/TheBinder/workflows/build-windows/badge.svg)![build-unix](https://github.com/giordi91/TheBinder/workflows/build-unix/badge.svg) 

Develop: ![build-windows](https://github.com/giordi91/TheBinder/workflows/build-windows/badge.svg?branch=develop)![build-unix](https://github.com/giordi91/TheBinder/workflows/build-unix/badge.svg?branch=develop)

This is a scripting language written mostly as a learning project following https://craftinginterpreters.com/ book. Hopefully at one point I would like to use it in my game engine to **bind** the gameplay and backend.

![alt text](docs/binderVM.png "binder screenshot")

## Goals
This projects comes with few goals I set for myself. Other than having a language I can mess around with, I had few goals in mind:
- Trying to to use STL. Not for any puristic reason or anything, but trying to take ownership of every evey aspect of the project (fro example memory allocations) and trying to keep compile times under control
- Mono build. I never used or tried a mono build, so this time I wanted to give it a shot, CMake is still used for the CI but main development is done using simple bat files
- WASM. I wanted to publish the language on the browser by cross compiling C++ to WASM.
- Trying a different workflow. Using a monobuild, simplified a lot the workflow and allowed me to move away from Visual Studio and try different workflow. In this case I wanted to try a worlflow based on VIM + Windows Terminal + Clang + RemedyBG. Again, not for any holy reason, just to try something a bit differnt and lear new tools like a new debugger. So far it has been good and I might be writing about it in my blog.

## Current status
As of now the project is up and running at : https://giordi91.github.io/pages/bind.html . 
I am pretty happy with the result so far, altough there is still so much work to be done. As usual is a learning project, altough I am trying to keep it up to standard I use in production I might cut some corners and leave some sharp edges.
Keep that in mind while browsing the code. 

The code is pretty much split in two parts (as does the book), a "legacyAST" and "vm" part.
The cleaner and faster part is the Virtual Machine part, which can be found under the "vm" folder. If you are looking at the code please check that out and ignore the ```legacyAST```.
The ```legacyAST``` folder was an initial attempt at implementing the Java I was learning form the book into C++. It was a reactive workflow which did not leave room for proper planning. As such I am not happy with it and parked it for the time being. I am not deleting it because there is a lot of useful stuff in there that I will need at one point to cleanup and refactor to make a multi pass, optimizing compiler. There are a lot of bad things in there like unhandled allocations/deallocations due to trying to first get a sense of the memory patterns and then devise a correct allocation strategy, you have been warned.

## Build
To build there are several options, the project has been created with a monobuild in mind, as such, it should be fairly trivial to build with your own scripts.

### Build c++
If you want to target native execution you can build using **CMake** by going in the repository folder and then:

```bash
mkdir build
cd build
cmake ../
cmake --build .
```

This should be all you need to do, this is what the CI on Github actions uses to build.

If you wish to build directly using the compiler you can see how to do it by having a look at the *customBuild* folder, specifically the build-win.bat file. The mono build setup means you only ever need to build one file main.cpp. The project currently has no dependencies and should be trivial to build.

### Build WASM
To build WASM the easiest thing is to use the custom build script in customBuild/build-js.bat on Windows. I am more than happy to accept PRs for different targets.

The first step is to setup the shell to be able to resolve em++. To do so simply clone somewhere the emscripten repository by doing:

```bash
# Get the emsdk repo
git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory
cd emsdk
```

Once inside in the repository run:
```bash
# if you haven't installed the latest sdk before
./emsdk install latest

./emsdk activate latest

# for unix
source ./emsdk_env.sh

# for windows
./emsdk_env
```

Now the environment should be ready to build WASM, to verify it run

```bash
em++ --version
```

it should return the version of Emscripten

finally to build the js app, cd back in TheBinder repository and run

```bash
cd customBuild
# for unix
./build-js.sh

# for windows
build-js.bat
```

if everything works you should find the built files inside "build/mono/js"
