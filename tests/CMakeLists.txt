cmake_minimum_required(VERSION 3.11.0)

project(Tests)   

	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   #grouping objects by name in the IDE
    function(sourceGroupByFolder targetDir sourceFiles)
       get_filename_component(targetDirName ${targetDir} NAME)
       source_group(TREE ${targetDir} PREFIX ${targetDirName} FILES ${sourceFiles})
	endfunction(sourceGroupByFolder)

	function (setFolder buildTarget folder)
		set_target_properties(${buildTarget} PROPERTIES FOLDER ${folder})
	endfunction(setFolder)


	#NOTE: when passing arrays/lists to macro, to work 
	#properly put the variable in quotes "${MY_VAR}"
	MACRO(SET_AS_HEADERS HEADERS)
	set_source_files_properties(
		${HEADERS}
		PROPERTIES HEADER_FILE_ONLY TRUE
	 )
	ENDMACRO(SET_AS_HEADERS)

	SET(SUPPORTING_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/treeSizesAllocatorTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/stringPoolAllocatorTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/stackAllocatorTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/resizableVectorTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tokenTests.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/scannerTests.cpp"
	)

	SET_AS_HEADERS("${SUPPORTING_FILES}")

    include_directories(
						${CMAKE_CURRENT_SOURCE_DIR}/src/
						${CMAKE_SOURCE_DIR}/core/includes
	)
	file(COPY "testData" DESTINATION "${CMAKE_BINARY_DIR}/bin")


	#making sure to add the common cpp flags, that are defined in the main cpp file
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}")

    #adding the executable
    add_executable(${PROJECT_NAME} src/main.cpp ${SUPPORTING_FILES})
	SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
    target_link_libraries(${PROJECT_NAME} TheBinderCore)
	setFolder(${PROJECT_NAME} Tests)
	sourceGroupByFolder( ${CMAKE_CURRENT_SOURCE_DIR} "${SUPPORTING_FILES}" )
	sourceGroupByFolder( ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")


	#setting working directory
	set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>)

