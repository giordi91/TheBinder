cmake_minimum_required(VERSION 3.11.0)

project(TheBinderCore)   

	#NOTE: when passing arrays/lists to macro, to work 
	#properly put the variable in quotes "${MY_VAR}"
	MACRO(SET_AS_HEADERS HEADERS)
	set_source_files_properties(
		${HEADERS}
		PROPERTIES HEADER_FILE_ONLY TRUE
	 )
	ENDMACRO(SET_AS_HEADERS)

	SET(SUPPORTING_FILES 
	"includes/binder/constants.h"
	"includes/binder/interpreter.h"
	"includes/binder/scanner.h"
	"includes/binder/tokens.h"
	"includes/binder/memory/stackAllocator.h"
	"includes/binder/memory/threeSizesPool.h"
	"includes/binder/memory/stringPool.h"
	"includes/binder/memory/resizableVector.h"
	"includes/binder/memory/stringHashMap.h"
	"includes/binder/enviroment.h"

	"includes/binder/vm/chunk.h"
	"includes/binder/vm/common.h"
	"includes/binder/vm/compiler.h"
	"includes/binder/vm/debug.h"
	"includes/binder/vm/memory.h"
	"includes/binder/vm/object.h"
	"includes/binder/vm/value.h"
	"includes/binder/vm/vm.h"

	"src/vm/compiler.cpp"
	"src/vm/debug.cpp"
	"src/vm/object.cpp"
	"src/vm/value.cpp"
	"src/vm/vm.cpp"


	"src/interpreter.cpp"
	"src/scanner.cpp"
	"src/memory/stringPool.cpp"
	)
	SET_AS_HEADERS("${SUPPORTING_FILES}")

	add_compile_definitions(FARMHASH_NO_BUILTIN_EXPECT)
    include_directories(
						${CMAKE_CURRENT_SOURCE_DIR}/includes
	)

	#making sure to add the common cpp flags, that are defined in the main cpp file
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}")

    #adding the executable
    add_library(${PROJECT_NAME} STATIC src/main.cpp ${SUPPORTING_FILES})
	SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

	#setting working directory
	set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>)

