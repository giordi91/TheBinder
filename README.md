# TheBinder 
Master:  ![build-windows](https://github.com/giordi91/TheBinder/workflows/build-windows/badge.svg)![build-unix](https://github.com/giordi91/TheBinder/workflows/build-unix/badge.svg) 

Develop: ![build-windows](https://github.com/giordi91/TheBinder/workflows/build-windows/badge.svg?branch=develop)![build-unix](https://github.com/giordi91/TheBinder/workflows/build-unix/badge.svg?branch=develop)

This is a scripting language written mostly as a learning project following https://craftinginterpreters.com/ book. Hopefully at one point I would like to use it in my game engine to **bind** the gameplay and backend.

## Features
### WASM compilation
While working on the code I started messing around with Emscripten and decided that I would have a WASM path, such that I can easy share my code for people to try on the browser

[ **INSERT IMAGE HERE AFTER NEW CSS REWORK**]

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
./emsdk activate latest
./emsdk_env
```

Now the environment should be ready to build WASM, without closing the console, move to the binder repository inside customBuild and just run build-js.bat

f everything works you should find the built files inside "build/mono/js"
