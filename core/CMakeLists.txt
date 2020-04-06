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
	"src/scanner.h"
	"src/scanner.cpp"
	)
	SET_AS_HEADERS("${SUPPORTING_FILES}")

    include_directories(
						${CMAKE_CURRENT_SOURCE_DIR}
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

