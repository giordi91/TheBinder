cmake_minimum_required(VERSION 3.11.0)

project(Emscripten)   


SET( SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
	"${CMAKE_SOURCE_DIR}/core/src/main.cpp"
	)
core/src/main.cpp tools/emscripten/main.cpp

	SET_AS_HEADERS("${SUPPORTING_FILES}")

    include_directories(
						${CMAKE_SOURCE_DIR}/core/includes
	)
	if(MSVC)
		file(COPY "testData" DESTINATION "${CMAKE_BINARY_DIR}/bin")
	else()
		file(COPY "testData" DESTINATION "${CMAKE_BINARY_DIR}")
	endif(MSVC)

	SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/wasm)

	#making sure to add the common cpp flags, that are defined in the main cpp file

    #adding the executable
    add_executable(binder ${SOURCES})

    message(STATUS "Setting compilation target to WASM")
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm.js")
    set_target_properties(binder PROPERTIES LINK_FLAGS "-Wall -std-c++17 -s WASM=1 -s EXPORTED_FUNCTIONS='[_main]'"




