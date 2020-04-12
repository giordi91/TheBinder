cmake_minimum_required(VERSION 3.11.0)

project(TheBinder)
# Set global property (all targets are impacted)
#preventing in source build
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
        message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
    endif()


#enable c++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



#check the compiler
MESSAGE(STATUS "compiler id ${CMAKE_CXX_COMPILER_ID} ${MSVC}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -g3 -O0 -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}   -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}  /std:c++17 /W4 /arch:AVX2 /fp:fast /MP /DNOMINMAX")
endif()


#options
option(BUILD_TESTS "Wheter or not build on test" OFF)

#just an overal log of the passed options
MESSAGE( STATUS "Building with the following options")
MESSAGE( STATUS "BUILD TESTS:                    " ${BUILD_TESTS})


#subfolders
add_subdirectory(core)
if(${BUILD_TESTS})
	add_subdirectory(tests)
endif(${BUILD_TESTS})

